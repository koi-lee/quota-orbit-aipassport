"""Authenticated private LAN snapshot format shared with orbit_quota_model.h."""
import ipaddress, math, secrets, struct, time
from cryptography.hazmat.primitives.ciphers.aead import AESGCM
AAD_PREFIX = b'OrbitQuota-v1'.ljust(16, b'\0')
def validate_ip(value):
    ip = ipaddress.IPv4Address(value)
    if not ip.is_private or ip.is_loopback or ip.is_link_local or ip.is_unspecified or ip.is_multicast: raise ValueError('Use the device private Wi-Fi IPv4 address')
    return str(ip)
def payload(windows, now=None, sequence=None):
    if not 1 <= len(windows) <= 2: raise ValueError('No usable quota windows')
    data = struct.pack('>BQQB',1,int(time.time() if now is None else now),time.time_ns() if sequence is None else sequence,len(windows))
    for window in windows:
        minutes=window['windowDurationMins']; remaining=window['remainingPercent']; reset=window['resetsAt'] or 0
        if not isinstance(minutes,int) or not 1<=minutes<=525600: raise ValueError('Invalid window duration')
        if not isinstance(remaining,(float,int)) or not math.isfinite(remaining) or not 0<=remaining<=100: raise ValueError('Invalid remaining quota')
        if not isinstance(reset,int) or not 0<=reset<=4102444800: raise ValueError('Invalid reset time')
        data += struct.pack('>IBQ',minutes,int(round(remaining)),reset)
    return data.ljust(44,b'\0')
def encrypt(key, boot, windows):
    if len(key)!=32 or len(boot)!=16: raise ValueError('Invalid pairing material')
    nonce=secrets.token_bytes(12)
    return nonce + AESGCM(key).encrypt(nonce,payload(windows),AAD_PREFIX+boot)
