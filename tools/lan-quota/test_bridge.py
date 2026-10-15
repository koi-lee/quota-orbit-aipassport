import hashlib, hmac, io, json, os, tempfile, unittest
from pathlib import Path
from types import SimpleNamespace
from unittest.mock import patch
import bridge
class BridgeTests(unittest.TestCase):
    def test_discovery_verifies_device_key_proof(self):
        nonce=bytes(range(16));boot=bytes(range(16,32));key=bytes(range(32))
        proof=hmac.new(key,bridge.DISCOVERY_PREFIX+nonce+boot,hashlib.sha256).hexdigest()
        payload=json.dumps({'version':1,'nonce':nonce.hex(),'boot':boot.hex(),'proof':proof}).encode()
        class FakeOpener:
            def open(self,url,timeout):
                self.url=url;self.timeout=timeout;return io.BytesIO(payload)
        opener=FakeOpener()
        with patch('bridge.secrets.token_bytes',return_value=nonce),patch('bridge.make_opener',return_value=opener):
            self.assertTrue(bridge.discover_at('192.168.1.4',key))
        self.assertIn('nonce='+nonce.hex(),opener.url)
    def test_discovery_rejects_another_pairing_key(self):
        nonce=bytes(range(16));boot=bytes(range(16,32));key=bytes(range(32))
        proof=hmac.new(key,bridge.DISCOVERY_PREFIX+nonce+boot,hashlib.sha256).hexdigest()
        payload=json.dumps({'version':1,'nonce':nonce.hex(),'boot':boot.hex(),'proof':proof}).encode()
        class FakeOpener:
            def open(self,*args,**kwargs):return io.BytesIO(payload)
        with patch('bridge.secrets.token_bytes',return_value=nonce),patch('bridge.make_opener',return_value=FakeOpener()):
            self.assertFalse(bridge.discover_at('192.168.1.4',bytes(reversed(key))))
    def test_scan_is_limited_to_active_private_ethernet_interface(self):
        output=('en0: flags=8863<UP,BROADCAST,SMART,RUNNING,SIMPLEX,MULTICAST> mtu 1500\n'
                '\tinet 192.168.1.4 netmask 0xffffff00 broadcast 192.168.1.255\n'
                '\tstatus: active\n'
                'en1: flags=8863<UP,BROADCAST> mtu 1500\n'
                '\tinet 10.0.0.2 netmask 0xffffff00 broadcast 10.0.0.255\n'
                '\tstatus: active\n'
                'utun4: flags=8051<UP,POINTOPOINT,RUNNING> mtu 1380\n'
                '\tinet 100.64.0.1 --> 100.64.0.2 netmask 0xffffffff\n')
        hosts=bridge.parse_scan_hosts(output,preferred='en0')
        self.assertEqual(len(hosts),253)
        self.assertNotIn('192.168.1.4',hosts)
        self.assertNotIn('10.0.0.2',hosts)
        self.assertNotIn('100.64.0.1',hosts)
    def test_scan_skips_large_subnets_instead_of_silently_truncating(self):
        output=('en0: flags=8863<UP,BROADCAST> mtu 1500\n'
                '\tinet 10.2.3.4 netmask 0xfffff000 broadcast 10.2.15.255\n'
                '\tstatus: active\n')
        hosts=bridge.parse_scan_hosts(output)
        self.assertEqual(hosts,[])
    def test_scan_accepts_exactly_supported_slash_23(self):
        output=('en0: flags=8863<UP,BROADCAST> mtu 1500\n'
                '\tinet 10.2.2.4 netmask 0xfffffe00 broadcast 10.2.3.255\n'
                '\tstatus: active\n')
        hosts=bridge.parse_scan_hosts(output)
        self.assertEqual(len(hosts),509)
        self.assertNotIn('10.2.2.4',hosts)
    def test_find_device_requires_one_exact_pairing_key_match(self):
        with patch('bridge.discover_at',side_effect=lambda ip,key:ip=='192.168.1.9'):
            self.assertEqual(bridge.find_device(bytes(32),['192.168.1.2','192.168.1.9']),'192.168.1.9')
        with patch('bridge.discover_at',return_value=True):
            with self.assertRaisesRegex(RuntimeError,'No unique Passport'):
                bridge.find_device(bytes(32),['192.168.1.2','192.168.1.9'])
    def test_atomic_key_permissions(self):
        with tempfile.TemporaryDirectory() as folder:
            state=Path(folder)/'pairing.json'
            with patch('bridge.STATE',state):bridge.save_pair(bytes(32),'192.168.1.2','test-device')
            self.assertEqual(state.stat().st_mode & 0o777,0o600)
            self.assertEqual(json.loads(state.read_text())['serial'],'test-device')
    def test_existing_other_device_is_preserved(self):
        with tempfile.TemporaryDirectory() as folder:
            state=Path(folder)/'pairing.json';state.write_text('{"serial":"other-device"}')
            device=SimpleNamespace(device='test-port',vid=0x303a,pid=0x1001,serial_number='test-device')
            with patch('bridge.STATE',state),patch('bridge.list_ports.comports',return_value=[device]),patch('bridge.serial.Serial') as serial:
                with self.assertRaises(RuntimeError):bridge.pair(None)
                serial.assert_not_called()
            self.assertEqual(state.read_text(),'{"serial":"other-device"}')
    def test_pair_requires_matching_accepted_key(self):
        key=bytes(range(32));ack=b'ORBIT_PAIR_ACCEPTED '+hashlib.sha256(key).hexdigest().encode()+b'\r\nORBIT_STATUS paired 192.168.1.2\r\n'
        class FakeSerial:
            def open(self):pass
            def close(self):pass
            def write(self,data):self.sent=data
            def read(self,size):return ack
        with tempfile.TemporaryDirectory() as folder:
            state=Path(folder)/'pairing.json';device=SimpleNamespace(device='test-port',vid=0x303a,pid=0x1001,serial_number='test-device')
            with patch('bridge.STATE',state),patch('bridge.list_ports.comports',return_value=[device]),patch('bridge.serial.Serial',return_value=FakeSerial()),patch('bridge.secrets.token_bytes',return_value=key),patch('builtins.print'):bridge.pair(None)
            self.assertEqual(json.loads(state.read_text())['key'],key.hex())
if __name__=='__main__':unittest.main()
