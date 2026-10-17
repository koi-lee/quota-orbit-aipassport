"""Exercise a frozen candidate in preview mode without USB, pairing or login-item writes."""
import argparse
import json
from pathlib import Path
import re
import socket
import subprocess
import tempfile
import time
import urllib.error
import urllib.request


def smoke(app):
    exe = Path(app).resolve() / 'Contents/MacOS/StarshorePassport'
    opener = urllib.request.build_opener(urllib.request.ProxyHandler({}))
    with socket.socket() as sock:
        sock.bind(('127.0.0.1', 0))
        port = sock.getsockname()[1]
    url = f'http://127.0.0.1:{port}'
    process = None
    with tempfile.TemporaryFile() as log:
        def start():
            child = subprocess.Popen([str(exe), '--preview', '--no-browser', '--port', str(port)],
                                     stdout=log, stderr=log)
            try:
                for _ in range(150):
                    if child.poll() is not None:
                        raise RuntimeError('candidate exited before opening its preview server')
                    try:
                        with opener.open(url + '/status', timeout=.5) as response:
                            status = json.load(response)
                        if status.get('allow_device') is not False or status.get('syncing') is not False:
                            raise RuntimeError('preview did not disable real-device operations')
                        return child
                    except (OSError, ValueError):
                        time.sleep(.1)
                raise RuntimeError('candidate preview startup timed out')
            except BaseException:
                child.terminate()
                child.wait(timeout=10)
                raise

        def quit_app(child):
            with opener.open(url, timeout=3) as response:
                page = response.read().decode('utf-8')
            match = re.search("'X-Local-Token':'([^']+)'", page)
            if not match or not all(label in page for label in ('zh-CN', 'zh-TW', 'Quit assistant')):
                raise RuntimeError('missing local action token or three-language UI')
            request = urllib.request.Request(url + '/action', data=b'{"action":"quit"}',
                headers={'Origin': url, 'X-Local-Token': match.group(1), 'Content-Type': 'application/json'})
            with opener.open(request, timeout=3) as response:
                if response.status != 200:
                    raise RuntimeError('quit action failed')
            if child.wait(timeout=10) != 0:
                raise RuntimeError('candidate exited with an error')
            try:
                opener.open(url + '/status', timeout=.5).close()
            except OSError:
                return
            raise RuntimeError('candidate left the preview port open')

        try:
            bridge = subprocess.run([str(exe), '--bridge', '--help'], stdout=log, stderr=log, timeout=20)
            if bridge.returncode != 0:
                raise RuntimeError('frozen bridge import/help failed')
            process = start()
            duplicate = subprocess.run([str(exe), '--preview', '--no-browser', '--port', str(port)],
                                       stdout=log, stderr=log, timeout=20)
            if duplicate.returncode != 0 or process.poll() is not None:
                raise RuntimeError('duplicate launch did not leave the original preview running')
            quit_app(process)
            process = None
            process = start()
            quit_app(process)
            process = None
        finally:
            if process is not None and process.poll() is None:
                process.terminate()
                process.wait(timeout=10)
    return {'bridge_import': True, 'preview_device_disabled': True, 'three_languages': True,
            'duplicate_launch': True, 'quit_releases_port': True, 'reopen': True,
            'usb_or_real_sync_tested': False}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('app', type=Path)
    parser.add_argument('--report', type=Path)
    args = parser.parse_args()
    result = smoke(args.app)
    text = json.dumps(result, indent=2) + '\n'
    if args.report:
        args.report.write_text(text)
    print(text, end='')


if __name__ == '__main__':
    main()
