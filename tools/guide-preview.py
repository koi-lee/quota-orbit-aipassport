#!/usr/bin/env python3
"""Local-only review server for the real LVGL renderer; never opens a USB device."""
import argparse
import json
import subprocess
import threading
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path
from PIL import Image

ROOT = Path(__file__).resolve().parents[1]
parser = argparse.ArgumentParser()
parser.add_argument('--port', type=int, default=8978)
parser.add_argument('--output', type=Path, required=True)
parser.add_argument('--renderer', type=Path, default=ROOT/'build/guide-render/guide_render')
parser.add_argument('--template', type=Path, default=ROOT/'tests/guide_render/preview.html')
parser.add_argument('--audio-dir', type=Path)
args = parser.parse_args()
args.output.mkdir(parents=True, exist_ok=True)
lock = threading.Lock()
actions = ''


def render():
    result = subprocess.run([str(args.renderer.resolve()), str(args.output),
                             str(ROOT/'assets/fonts/guide-characters.txt'), actions],
                            check=True, text=True, capture_output=True, timeout=10)
    Image.open(args.output/'live.ppm').save(args.output/'live.png')
    return json.loads(result.stdout.splitlines()[-1])


class Handler(BaseHTTPRequestHandler):
    def send(self, content, kind='application/json', status=200):
        if isinstance(content, str):
            content = content.encode()
        self.send_response(status)
        self.send_header('Content-Type', kind)
        self.send_header('Content-Length', str(len(content)))
        self.send_header('Cache-Control', 'no-store')
        self.end_headers()
        self.wfile.write(content)

    def do_GET(self):
        if self.path == '/':
            self.send(args.template.read_bytes(), 'text/html; charset=utf-8')
        elif self.path.startswith('/frame.png'):
            with lock:
                self.send((args.output/'live.png').read_bytes(), 'image/png')
        elif self.path.startswith('/voice/') and args.audio_dir:
            name=self.path.removeprefix('/voice/')
            if name not in [f'clip_{i:02}.wav' for i in range(22)]:
                self.send('Not found','text/plain',404)
                return
            self.send((args.audio_dir/name).read_bytes(),'audio/wav')
        elif self.path == '/state':
            with lock:
                self.send(json.dumps(render()))
        else:
            self.send('Not found', 'text/plain', 404)

    def do_POST(self):
        global actions
        # Only the local preview origin may drive this in-memory test session.
        if self.headers.get('Origin') != f'http://127.0.0.1:{args.port}':
            self.send('Forbidden', 'text/plain', 403)
            return
        if self.path != '/key' or int(self.headers.get('Content-Length', '0')) > 128:
            self.send('Bad request', 'text/plain', 400)
            return
        try:
            key = json.loads(self.rfile.read(int(self.headers['Content-Length'])))['key']
            if key not in ('u', 'd', 'o', 'b', 'reset', 'ended', 'failed'):
                raise ValueError('Invalid key')
            with lock:
                if key == 'reset':
                    actions = ''
                elif len(actions) < 4096:
                    actions += {'ended':'e','failed':'f'}.get(key,key)
                else:
                    raise ValueError('Reset preview to continue')
                self.send(json.dumps(render()))
        except (ValueError, KeyError, subprocess.SubprocessError) as error:
            self.send(json.dumps({'error': str(error)}), status=400)

    def log_message(self, *_):
        pass


render()
print(f'Pocket Guide native UI preview: http://127.0.0.1:{args.port}/', flush=True)
ThreadingHTTPServer(('127.0.0.1', args.port), Handler).serve_forever()
