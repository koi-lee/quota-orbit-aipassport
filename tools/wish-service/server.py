#!/usr/bin/env python3
"""Loopback ingress for anonymous Passport events, sharing the website counter DB."""
import argparse
from collections import OrderedDict
from contextlib import closing
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
import json
from pathlib import Path
import sqlite3
import threading
import time
import uuid

ROUTE = '/quota-orbit/api/passport/wishes'
MAX_BATCH = 32

class Store:
    def __init__(self, path):
        self.path = Path(path)
        if not self.path.is_file():
            raise ValueError('Existing shared counter database required')
        with closing(sqlite3.connect(self.path)) as db:
            columns = db.execute('PRAGMA table_info(clicks)').fetchall()
            names = {c[1] for c in columns}
            if names not in ({'id', 'received_at', 'version'}, {'id', 'received_at', 'version', 'country'}):
                raise ValueError('Unverified shared schema; refusing to write')
            if 'country' in names and any(c[1] == 'country' and (c[3] or c[4] is not None) for c in columns):
                raise ValueError('Country column must be nullable without a default')
            if not any(c[1] == 'id' and c[5] == 1 for c in columns):
                raise ValueError('Event id primary key required')
    def accept(self, events):
        if not isinstance(events, list) or not 1 <= len(events) <= MAX_BATCH:
            raise ValueError('Invalid batch')
        for event in events:
            if not isinstance(event, str) or str(uuid.UUID(event, version=4)) != event:
                raise ValueError('Invalid event id')
        if len(set(events)) != len(events):
            raise ValueError('Duplicate ids inside batch')
        with closing(sqlite3.connect(self.path, timeout=1)) as db:
            db.execute('BEGIN IMMEDIATE')
            db.executemany('INSERT OR IGNORE INTO clicks (id,received_at,version) VALUES (?,?,2)',
                           [(event, int(time.time())) for event in events])
            total = db.execute('SELECT COUNT(*) FROM clicks').fetchone()[0]
            db.commit()
        return {'total': total, 'accepted': events}

class Limit:
    def __init__(self):
        self.entries = OrderedDict()
        self.lock = threading.Lock()
    def allow(self, key):
        with self.lock:
            now = time.monotonic()
            for old in list(self.entries):
                if now - self.entries[old][1] > 60:
                    del self.entries[old]
            tokens, at = self.entries.get(key, (6.0, now))
            if key not in self.entries and len(self.entries) >= 1024:
                return False
            tokens = min(6.0, tokens + (now - at) * 2)
            ok = tokens >= 1
            self.entries[key] = (tokens - 1 if ok else tokens, now)
            return ok

class Server(ThreadingHTTPServer):
    daemon_threads = True
    def __init__(self, address, store):
        self.store, self.limit = store, Limit()
        super().__init__(address, Handler)

class Handler(BaseHTTPRequestHandler):
    def setup(self):
        super().setup()
        self.connection.settimeout(5)
    def log_message(self, *args):
        pass  # Do not log anonymous event ids or client addresses.
    def reply(self, code, data):
        body = json.dumps(data, separators=(',', ':')).encode()
        self.send_response(code)
        self.send_header('Content-Type', 'application/json')
        self.send_header('Cache-Control', 'no-store')
        self.send_header('Content-Length', str(len(body)))
        if code == 429:
            self.send_header('Retry-After', '5')
        self.end_headers()
        self.wfile.write(body)
    def do_POST(self):
        if self.path != ROUTE:
            return self.reply(404, {'error': 'route'})
        if not self.server.limit.allow(self.client_address[0]):
            return self.reply(429, {'error': 'rate'})
        if self.headers.get('Transfer-Encoding') or len(self.headers.get_all('Content-Length', [])) != 1:
            return self.reply(400, {'error': 'framing'})
        try:
            size = int(self.headers.get('Content-Length', '0'))
            if not 0 < size <= 2048:
                return self.reply(413, {'error': 'size'})
            if self.headers.get('Content-Type', '').split(';')[0].strip() != 'application/json':
                return self.reply(415, {'error': 'type'})
            data = json.loads(self.rfile.read(size))
            if not isinstance(data, dict) or set(data) != {'events'}:
                raise ValueError('fields')
            answer = self.server.store.accept(data['events'])
        except (ValueError, UnicodeError):
            return self.reply(400, {'error': 'events'})
        except sqlite3.Error:
            return self.reply(503, {'error': 'storage'})
        self.reply(200, answer)
    def do_GET(self):
        self.reply(405, {'error': 'method'})
    do_OPTIONS = do_GET

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--db', required=True, type=Path)
    parser.add_argument('--port', type=int, default=8970)
    args = parser.parse_args()
    Server(('127.0.0.1', args.port), Store(args.db)).serve_forever()
