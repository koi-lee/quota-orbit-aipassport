import unittest, struct
from cryptography.hazmat.primitives.ciphers.aead import AESGCM
from cryptography.exceptions import InvalidTag
from protocol import payload, encrypt, AAD_PREFIX, validate_ip
class ProtocolTests(unittest.TestCase):
    def setUp(self):self.windows=[{'remainingPercent':42,'windowDurationMins':10080,'resetsAt':1800000000}]
    def test_missing_window_not_filled(self):
        data=payload(self.windows,1800000000,1)
        self.assertEqual(len(data),44);self.assertEqual(data[17],1);self.assertEqual(data[31:],b'\0'*13)
        self.assertEqual(struct.unpack('>IBQ',data[18:31]),(10080,42,1800000000))
    def test_crypto_tamper_wrong_key_and_boot(self):
        key=bytes(range(32));boot=bytes(range(16));packet=encrypt(key,boot,self.windows)
        self.assertEqual(len(packet),72)
        self.assertEqual(len(AESGCM(key).decrypt(packet[:12],packet[12:],AAD_PREFIX+boot)),44)
        for different_key,different_boot,body in [(bytes(32),boot,packet[12:]),(key,bytes(16),packet[12:]),(key,boot,packet[12:-1]+bytes([packet[-1]^1]))]:
            with self.assertRaises(InvalidTag):AESGCM(different_key).decrypt(packet[:12],body,AAD_PREFIX+different_boot)
    def test_nonce_unique_and_unknown_rejected(self):
        key=bytes(32);boot=bytes(16)
        self.assertNotEqual(encrypt(key,boot,self.windows)[:12],encrypt(key,boot,self.windows)[:12])
        with self.assertRaises(ValueError):payload([])
        with self.assertRaises(ValueError):payload([dict(self.windows[0],remainingPercent=float('nan'))])
        for ip in ['127.0.0.1','8.8.8.8','0.0.0.0','224.0.0.1']:
            with self.assertRaises(ValueError):validate_ip(ip)
        self.assertEqual(validate_ip('192.168.1.10'),'192.168.1.10')
if __name__=='__main__':unittest.main()
