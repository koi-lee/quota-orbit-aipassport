import concurrent.futures
from contextlib import closing
import http.client
from pathlib import Path
import sqlite3
import tempfile
import threading
import unittest
import uuid
import json
import ctypes
import hashlib
import os
import subprocess
import server

class Wishes(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.db = Path(self.temp.name) / 'wishes.db'
        with closing(sqlite3.connect(self.db)) as conn:
            conn.execute('CREATE TABLE clicks (id TEXT PRIMARY KEY, received_at INTEGER NOT NULL, version INTEGER NOT NULL)')
            conn.execute('INSERT INTO clicks VALUES (?,1,1)', (str(uuid.uuid4()),))
            conn.commit()
        self.store = server.Store(self.db)
    def tearDown(self):
        self.temp.cleanup()
    def total(self):
        with closing(sqlite3.connect(self.db)) as conn:
            return conn.execute('SELECT COUNT(*) FROM clicks').fetchone()[0]
    def test_shared_counter_retry_and_reopen(self):
        events = [str(uuid.uuid4()) for _ in range(20)]
        self.assertEqual(self.store.accept(events)['total'], 21)
        # Lost response: same ids after restart must not count again.
        self.assertEqual(server.Store(self.db).accept(events)['total'], 21)
        self.assertEqual(self.total(), 21)  # Same SELECT as the website.
        self.assertEqual(self.store.accept([str(uuid.uuid4()) for _ in range(3)])['total'], 24)
    def test_current_country_schema_preserves_website_history(self):
        with closing(sqlite3.connect(self.db)) as conn:
            conn.execute('ALTER TABLE clicks ADD COLUMN country TEXT')
            conn.execute("UPDATE clicks SET country='CN'")
            conn.commit()
        store = server.Store(self.db)
        events = [str(uuid.uuid4()) for _ in range(20)]
        self.assertEqual(store.accept(events)['total'], 21)
        with closing(sqlite3.connect(self.db)) as conn:
            total, cursor = conn.execute('SELECT COUNT(*),COALESCE(MAX(rowid),0) FROM clicks').fetchone()
            self.assertEqual((total, cursor), (21, 21))
            self.assertEqual(conn.execute("SELECT COUNT(*) FROM clicks WHERE country='CN'").fetchone()[0], 1)
            self.assertEqual(conn.execute('SELECT COUNT(*) FROM clicks WHERE country IS NULL').fetchone()[0], 20)
        self.assertEqual(store.accept(events)['total'], 21)

    def test_concurrent_retries(self):
        events = [str(uuid.uuid4()) for _ in range(32)]
        with concurrent.futures.ThreadPoolExecutor(max_workers=6) as pool:
            list(pool.map(self.store.accept, [events] * 6))
        self.assertEqual(self.total(), 33)
    def test_invalid_or_full_batch_does_not_write(self):
        for events in ([], ['invalid'], ['00000000-0000-0000-0000-000000000000'],
                       [str(uuid.uuid4())] * 2, [str(uuid.uuid4()) for _ in range(33)]):
            with self.assertRaises(ValueError):
                self.store.accept(events)
        self.assertEqual(self.total(), 1)
    def test_unverified_database_fails_closed(self):
        with self.assertRaises(ValueError): server.Store(Path(self.temp.name) / 'missing')
        with closing(sqlite3.connect(self.db)) as conn:
            conn.execute('ALTER TABLE clicks ADD COLUMN unverified TEXT')
        with self.assertRaises(ValueError): server.Store(self.db)
    def test_firmware_model_to_shared_counter(self):
        root = Path(__file__).resolve().parents[2]
        libpath = Path(self.temp.name) / 'wish_model.so'
        subprocess.run([os.environ.get('CC', 'cc'), '-shared', '-fPIC', '-std=c11',
                        '-I' + str(root / 'main'), str(root / 'tests/wish_model_bridge.c'),
                        '-o', str(libpath)], check=True)
        model = ctypes.CDLL(str(libpath))
        model.wish_size.restype = ctypes.c_size_t
        model.wish_first.restype = ctypes.c_uint64
        model.wish_ack.argtypes = [ctypes.c_void_p, ctypes.c_uint64, ctypes.c_uint]
        size = model.wish_size()
        queue = ctypes.create_string_buffer(size)
        seed = bytes(range(32))
        self.assertTrue(model.wish_enable(queue, seed))
        for _ in range(20): self.assertTrue(model.wish_add(queue))
        persisted = bytes(queue)
        http = server.Server(('127.0.0.1', 0), self.store)
        thread = threading.Thread(target=http.serve_forever, daemon=True); thread.start()
        def send(snapshot):
            first = model.wish_first(snapshot)
            count = min(model.wish_pending(snapshot), 32)
            ids = [str(uuid.UUID(bytes=hashlib.sha256(seed + i.to_bytes(8, 'big')).digest()[:16], version=4))
                   for i in range(first, first + count)]
            conn = http_client.HTTPConnection('127.0.0.1', http.server_port, timeout=2)
            conn.request('POST', server.ROUTE, json.dumps({'events': ids}), {'Content-Type': 'application/json'})
            answer = conn.getresponse(); self.assertEqual(answer.status, 200)
            result = json.loads(answer.read()); conn.close()
            self.assertEqual(result['accepted'], ids)
            return first, count, result['total']
        import http.client as http_client
        try:
            first, count, total = send(queue)
            self.assertEqual(total, 21)
            # Lose the ACK and reboot from the durable C state; event ids stay stable.
            queue = ctypes.create_string_buffer(persisted, size)
            self.assertEqual(send(queue)[2], 21)
            self.assertTrue(model.wish_ack(queue, first, count))
            self.assertEqual(model.wish_pending(queue), 0)
            for _ in range(3): self.assertTrue(model.wish_add(queue))
            queue = ctypes.create_string_buffer(bytes(queue), size)
            first, count, total = send(queue)
            self.assertEqual(total, 24)
            self.assertTrue(model.wish_ack(queue, first, count))
            model.wish_add(queue); model.wish_disable(queue)
            self.assertFalse(model.wish_add(queue))
            self.assertEqual(model.wish_pending(queue), 0)
            self.assertEqual(self.total(), 24)
        finally:
            http.shutdown(); http.server_close(); thread.join()

    def test_http_and_rate_limit(self):
        http = server.Server(('127.0.0.1', 0), self.store)
        thread = threading.Thread(target=http.serve_forever, daemon=True); thread.start()
        def request(data, method='POST', content_type='application/json'):
            conn = http_client()
            conn.request(method, server.ROUTE, data, {'Content-Type': content_type})
            response = conn.getresponse(); body = response.read(); status = response.status; conn.close()
            return status, body
        def http_client():
            return http_module.HTTPConnection('127.0.0.1', http.server_port, timeout=2)
        import http.client as http_module
        try:
            payload=json.dumps({'events':[str(uuid.uuid4())]})
            self.assertEqual(request(payload)[0], 200)
            self.assertEqual(request(payload)[0], 200)
            self.assertEqual(self.total(), 2)
            self.assertEqual(request('{broken')[0], 400)
            self.assertEqual(request(json.dumps({'events':['bad'],'account':'never send'}))[0], 400)
            self.assertEqual(request(payload, content_type='text/plain')[0], 415)
            self.assertEqual(request(payload, method='GET')[0], 405)
            self.assertEqual(request(payload)[0], 200)
            self.assertEqual(request(payload)[0], 429)
        finally:
            http.shutdown(); http.server_close(); thread.join()

if __name__ == '__main__': unittest.main()
