"""Read-only local Codex quota probe; never exports credentials or raw RPC payloads."""
import json, math, os, selectors, shutil, subprocess, time
from pathlib import Path

def codex_executable():
    # Login agents do not inherit the interactive shell's Homebrew PATH.
    candidates = [shutil.which('codex'), '/opt/homebrew/bin/codex',
                  '/usr/local/bin/codex', '/Applications/Codex.app/Contents/Resources/codex',
                  str(Path.home() / 'Applications/Codex.app/Contents/Resources/codex')]
    for candidate in candidates:
        if candidate and Path(candidate).is_file() and os.access(candidate, os.X_OK):
            return candidate
    raise FileNotFoundError('Codex executable unavailable')

def codex_environment():
    env = os.environ.copy()
    paths = env.get('PATH', '/usr/bin:/bin:/usr/sbin:/sbin').split(os.pathsep)
    # npm's Codex launcher also needs Node on PATH in a login-agent session.
    for directory in ['/opt/homebrew/bin', '/usr/local/bin']:
        if directory not in paths: paths.append(directory)
    env['PATH'] = os.pathsep.join(paths)
    return env

def read_quota(timeout=15):
    process = subprocess.Popen([codex_executable(), 'app-server'], env=codex_environment(), stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.DEVNULL, bufsize=0)
    selector = selectors.DefaultSelector(); selector.register(process.stdout, selectors.EVENT_READ)
    pending = b''
    def send(value):
        process.stdin.write((json.dumps(value) + '\n').encode()); process.stdin.flush()
    def response(request_id):
        nonlocal pending
        until = time.monotonic() + timeout
        while time.monotonic() < until:
            while b'\n' in pending:
                line, pending = pending.split(b'\n', 1)
                value = json.loads(line)
                if value.get('id') == request_id:
                    if 'error' in value: raise RuntimeError('Codex rejected read-only RPC')
                    return value.get('result', {})
            if not selector.select(max(0, until-time.monotonic())): break
            chunk = os.read(process.stdout.fileno(), 4096)
            if not chunk: break
            pending += chunk
            if len(pending) > 1048576: raise RuntimeError('RPC response too large')
        raise TimeoutError('Codex read-only RPC timeout')
    try:
        send({'id':1,'method':'initialize','params':{'clientInfo':{'name':'starshore_passport_probe','version':'0.1.0'}}})
        response(1); send({'method':'initialized'})
        send({'id':2,'method':'account/rateLimits/read'})
        result = response(2)
        return windows_from_result(result)
    finally:
        selector.close(); process.terminate()
        try: process.wait(timeout=3)
        except subprocess.TimeoutExpired: process.kill(); process.wait()
        process.stdin.close(); process.stdout.close()

def windows_from_result(result):
    buckets = result.get('rateLimitsByLimitId')
    bucket = buckets.get('codex') if isinstance(buckets,dict) and buckets else result.get('rateLimits')
    if not isinstance(bucket,dict) or bucket.get('limitId','codex') != 'codex': return []
    return [{'remainingPercent':max(0,min(100,100-window['usedPercent'])), 'windowDurationMins':window.get('windowDurationMins'), 'resetsAt':window.get('resetsAt')}
            for name in ['primary','secondary'] if isinstance(window:=bucket.get(name),dict) and type(window.get('usedPercent')) in (int,float) and math.isfinite(window['usedPercent'])]

if __name__ == '__main__':
    try:
        windows = read_quota()
        report = {'rpc':'account/rateLimits/read','success':bool(windows),'window_count':len(windows),'window_duration_minutes':[w['windowDurationMins'] for w in windows], 'credentials_exported':False,'personal_values_saved':False}
    except Exception as error:
        report = {'success':False,'error_type':type(error).__name__,'credentials_exported':False,'personal_values_saved':False}
    out=Path('outputs/lan-quota-20261014');out.mkdir(parents=True,exist_ok=True)
    (out/'probe-result.json').write_text(json.dumps(report,indent=2))
    print(json.dumps(report))
