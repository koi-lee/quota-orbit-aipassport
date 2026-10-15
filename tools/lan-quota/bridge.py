"""USB-confirmed pairing, then encrypted quota pushes over the user's local Wi-Fi."""
import argparse, concurrent.futures, hashlib, hmac, ipaddress, json, os, re, secrets, socket, subprocess, time, urllib.error, urllib.parse, urllib.request
from pathlib import Path
import serial
from serial.tools import list_ports
from probe import read_quota
from protocol import encrypt, validate_ip
STATE = Path.home()/ 'Library/Application Support/StarshorePassport/pairing.json'
DISCOVERY_PREFIX = b'OrbitDiscover-v1'
def save_pair(key,ip,serial_number):
    STATE.parent.mkdir(parents=True,exist_ok=True)
    temp=STATE.with_suffix('.'+secrets.token_hex(8)+'.new')
    fd=os.open(temp,os.O_WRONLY|os.O_CREAT|os.O_EXCL,0o600)
    with os.fdopen(fd,'w') as file: json.dump({'key':key.hex(),'ip':validate_ip(ip) if ip else None,'serial':serial_number},file)
    temp.replace(STATE)
def pair(target):
    ports=[p for p in list_ports.comports() if p.vid==0x303a and p.pid==0x1001 and (not target or p.device==target)]
    if len(ports)!=1: raise RuntimeError('Connect exactly one Passport, or provide its explicit port')
    port=ports[0]
    if not port.serial_number: raise RuntimeError('Device identity unavailable; do not pair an ambiguous target')
    if STATE.exists() and json.loads(STATE.read_text()).get('serial') != port.serial_number:
        raise RuntimeError('This helper is paired to a different device; existing authorization was preserved')
    key=secrets.token_bytes(32)
    device=serial.Serial();device.port=port.device;device.baudrate=115200;device.timeout=.2;device.dtr=False;device.rts=False;device.open()
    try:
        print('USB连接可能让设备回到首页。请现在在硬件长按确定 → 设置 → 连接 Mac，再按确定允许。配对期间不要断开USB。',flush=True)
        end=time.monotonic()+120;pending=b'';next_offer=0;accepted=False
        while time.monotonic()<end:
            if time.monotonic()>=next_offer:
                device.write(b'ORBIT_PAIR '+key.hex().encode()+b'\nORBIT_STATUS\n'); next_offer=time.monotonic()+2
            pending+=device.read(2048)
            while b'\n' in pending:
                line,pending=pending.split(b'\n',1)
                # ESP-IDF's console emits CRLF; normalize framing before exact ACK verification.
                line=line.rstrip(b'\r')
                if b'ESP-ROM:' in line:
                    print('设备已重启，请现在在硬件长按确定 → 设置 → 连接 Mac。',flush=True)
                if line == b'ORBIT_PAIR_OFFER':
                    print('硬件已收到请求，请在设备上按确定允许同步。',flush=True)
                if line == b'ORBIT_PAIR_ACCEPTED '+hashlib.sha256(key).hexdigest().encode():
                    accepted=True; save_pair(key,None,port.serial_number)
                if accepted and line.startswith(b'ORBIT_STATUS paired '):
                    ip=line.split()[-1].decode()
                    if ip == 'offline':continue
                    validate_ip(ip)
                    save_pair(key,ip,port.serial_number); print('配对完成。Wi-Fi同步可在拔掉USB后继续。');return
            if len(pending)>4096:pending=pending[-4096:]
        raise TimeoutError('Device confirmed but Wi-Fi address unavailable; key saved, use --ip after reconnect' if accepted else 'Pairing timed out; no new key saved')
    finally: device.close()
def make_opener():
    # Ignore system proxy settings for the explicit private IPv4 destination.
    class NoRedirect(urllib.request.HTTPRedirectHandler):
        def redirect_request(self,*args,**kwargs):return None
    return urllib.request.build_opener(urllib.request.ProxyHandler({}),NoRedirect())
def discover_at(ip,key,timeout=.6):
    ip=validate_ip(ip);nonce=secrets.token_bytes(16)
    url=f'http://{ip}:8899/discover?nonce={nonce.hex()}'
    with make_opener().open(url,timeout=timeout) as response:result=json.loads(response.read(256))
    if result.get('version')!=1 or result.get('nonce')!=nonce.hex():return False
    try:boot=bytes.fromhex(result['boot']);proof=bytes.fromhex(result['proof'])
    except (KeyError,ValueError,TypeError):return False
    expected=hmac.new(key,DISCOVERY_PREFIX+nonce+boot,hashlib.sha256).digest()
    return len(boot)==16 and hmac.compare_digest(proof,expected)
def parse_scan_hosts(ifconfig_output,preferred=None):
    blocks=re.split(r'(?m)(?=^[A-Za-z][A-Za-z0-9._-]*: flags=)',ifconfig_output)
    hosts=set()
    for block in blocks:
        match=re.match(r'^([A-Za-z][A-Za-z0-9._-]*): flags=([^\n]+)',block)
        if not match:continue
        interface,flags=match.groups()
        if not interface.startswith('en') or 'UP' not in flags or 'status: inactive' in block:continue
        if preferred and interface!=preferred:continue
        address=re.search(r'(?m)^\s+inet ([0-9.]+) netmask (0x[0-9a-fA-F]+)',block)
        if not address:continue
        try:
            ip=ipaddress.IPv4Address(address.group(1))
            mask=str(ipaddress.IPv4Address(int(address.group(2),16)))
            network=ipaddress.IPv4Network((ip,mask),strict=False)
            if not ip.is_private or network.prefixlen<23:continue
            hosts.update(str(candidate) for candidate in network.hosts() if candidate!=ip)
        except (ValueError,OverflowError):continue
    return sorted(hosts,key=lambda value:int(ipaddress.IPv4Address(value)))
def scan_hosts():
    preferred=None
    try:
        route=subprocess.run(['route','-n','get','default'],capture_output=True,text=True,timeout=2,check=False).stdout
        match=re.search(r'(?m)^\s*interface:\s*(\S+)',route)
        if match and match.group(1).startswith('en'):preferred=match.group(1)
    except (OSError,subprocess.TimeoutExpired):pass
    try:output=subprocess.run(['ifconfig','-a'],capture_output=True,text=True,timeout=2,check=True).stdout
    except (OSError,subprocess.SubprocessError):return []
    return parse_scan_hosts(output,preferred)
def find_device(key,hosts=None):
    hosts=scan_hosts() if hosts is None else hosts
    if not hosts:raise RuntimeError('No local Wi-Fi hosts are available for device discovery')
    matches=[]
    with concurrent.futures.ThreadPoolExecutor(max_workers=32) as pool:
        futures={pool.submit(discover_at,ip,key):ip for ip in hosts}
        for future in concurrent.futures.as_completed(futures):
            try:
                if future.result():matches.append(futures[future])
            except (OSError,ValueError,urllib.error.URLError,urllib.error.HTTPError,TimeoutError,json.JSONDecodeError):pass
    if len(matches)!=1:raise RuntimeError('No unique Passport with this pairing key was found on the local Wi-Fi')
    return matches[0]
def select_device(config,key):
    saved=validate_ip(config['ip']) if config.get('ip') else None
    if saved:
        try:
            if discover_at(saved,key):return saved
        except urllib.error.HTTPError as error:
            if error.code==404:return saved # Older paired firmware has no discovery endpoint.
        except (OSError,ValueError,urllib.error.URLError,TimeoutError,json.JSONDecodeError):pass
    return find_device(key)
def push_to_device(ip,key,windows):
    opener=make_opener()
    with opener.open(f'http://{ip}:8899/challenge',timeout=3) as response: challenge=json.loads(response.read(256))
    if challenge.get('version')!=1 or not challenge.get('paired'):raise RuntimeError('Device not paired; confirm USB pairing again')
    boot=bytes.fromhex(challenge['boot'])
    packet=encrypt(key,boot,windows)
    request=urllib.request.Request(f'http://{ip}:8899/quota',data=packet,headers={'Content-Type':'application/octet-stream'},method='POST')
    with opener.open(request,timeout=3) as response:
        if response.read(16)!=b'OK':raise RuntimeError('Device did not accept quota')
def push_once():
    config=json.loads(STATE.read_text());key=bytes.fromhex(config['key'])
    ip=select_device(config,key)
    if config.get('ip')!=ip:save_pair(key,ip,config['serial'])
    windows=read_quota()
    try:push_to_device(ip,key,windows)
    except (OSError,urllib.error.URLError,urllib.error.HTTPError,TimeoutError,socket.timeout):
        replacement=find_device(key)
        if replacement==ip:raise
        save_pair(key,replacement,config['serial'])
        push_to_device(replacement,key,windows)
    print('个人额度同步完成（未打印个人数值）',flush=True)
def main():
    parser=argparse.ArgumentParser();parser.add_argument('command',choices=['pair','run','once','forget']);parser.add_argument('--port');parser.add_argument('--ip');args=parser.parse_args()
    if args.command=='pair':pair(args.port);return
    if args.command=='forget':
        print('请先在硬件设置中解除 Mac；这使设备立即撤销授权。')
        if STATE.exists():STATE.unlink()
        return
    if args.ip:
        config=json.loads(STATE.read_text());save_pair(bytes.fromhex(config['key']),args.ip,config['serial'])
    while True:
        try:push_once()
        except Exception as error:
            print('同步未完成：'+type(error).__name__+'。设备将标记旧数据；检查Mac登录、Wi-Fi或配对。',flush=True)
            if args.command=='once':raise SystemExit(1)
        if args.command=='once':return
        time.sleep(30)
if __name__=='__main__':main()
