"""Local Mac helper UI. UI binds loopback; private quota never enters project previews."""
import argparse, json, os, plistlib, secrets, subprocess, sys, tempfile, threading, time, webbrowser
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path
from urllib.parse import urlsplit
TOKEN=secrets.token_hex(24)
state={'busy':False,'syncing':False,'message':'waiting','updated_at':''}
lock=threading.Lock();stop=threading.Event()
ROOT=Path(__file__).resolve().parent
PORT=8942
SERVER=None
children=set()
sync_thread=None
PAGE='''<!doctype html><html lang="zh-CN"><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1"><title>隨身星盤 · Mac同步助手</title><style>body{background:#211a2b;color:#eee6f1;font:16px system-ui,-apple-system,sans-serif;margin:0;padding:24px}main{max-width:640px;margin:40px auto;padding:28px;border:1px solid #665676;border-radius:18px}header{display:flex;align-items:center;justify-content:space-between;gap:16px}h1{font-size:26px;line-height:1.3}select{font:14px system-ui;padding:9px;background:#30263c;color:#eee6f1;border:1px solid #665676;border-radius:8px}p{line-height:1.7}button{font:16px system-ui;padding:12px 18px;border:0;border-radius:9px;margin:7px;background:#e2c18b;color:#211a2b}button:disabled{opacity:.45}.hint{color:#bdaec9;font-size:14px}#status{background:#30263c;padding:16px;border-radius:10px;overflow-wrap:anywhere}.sr-only{position:absolute;width:1px;height:1px;padding:0;margin:-1px;overflow:hidden;clip:rect(0,0,0,0);white-space:nowrap;border:0}@media(max-width:560px){body{padding:12px}main{margin:12px auto;padding:18px}header{align-items:flex-start;flex-direction:column}h1{font-size:22px}}</style><main><header><h1 data-i18n="title">隨身星盤 · Mac同步助手</h1><label><span class="sr-only" data-i18n="languageLabel">語言</span><select id="language" aria-label="Language"><option value="zh-CN">简体中文</option><option value="zh-TW">繁體中文</option><option value="en">English</option></select></label></header><p data-i18n="flow">已登录 Codex 的 Mac → 同一 Wi-Fi → 你的 AI Passport</p><p data-i18n="setup">首次用 USB 数据线连接设备，先点击下方「连接设备」。设备可能重启回到首页；待启动后，长按硬件确定进入设置，选择「连接 Mac」，再在硬件按确定允许同步。配对成功后将自动同步，无需每次手动点击。</p><div id="status" role="status" aria-live="polite">正在读取助手状态…</div><button data-action="pair" data-i18n="pair">连接设备</button><button data-action="start" data-i18n="start">恢复同步</button><button data-action="stop" data-i18n="pause">暂停同步</button><button id="quit" data-action="quit" data-i18n="quit">退出助手</button><p class="hint" data-i18n="hint">Mac登录后助手会自动启动并恢复同步。Mac需要保持运行；每30秒更新。断网或休眠时，设备显示旧额度和更新时间。可在系统设置→通用→登录项中关闭自动启动。登录凭据留在电脑；设备收到加密额度摘要。</p><p class="hint" data-i18n="revoke">解除配对：在硬件设置中选择「解除 Mac 配对」并确认。这会撤销授权并清除设备上的个人额度。</p></main><script>
const copy={
'zh-CN':{title:'随身星盘 · Mac同步助手',languageLabel:'语言',flow:'已登录 Codex 的 Mac → 同一 Wi-Fi → 你的 AI Passport',setup:'首次用 USB 数据线连接设备，先点击下方「连接设备」。设备可能重启回到首页；待启动后，长按硬件确定进入设置，选择「连接 Mac」，再在硬件按确定允许同步。配对成功后将自动同步，无需每次手动点击。',pair:'连接设备',start:'恢复同步',pause:'暂停同步',quit:'退出助手',hint:'Mac登录后助手会自动启动并恢复同步。Mac需要保持运行；每30秒更新。断网或休眠时，设备显示旧额度和更新时间。可在系统设置→通用→登录项中关闭自动启动。登录凭据留在电脑；设备收到加密额度摘要。',revoke:'解除配对：在硬件设置中选择「解除 Mac 配对」并确认。这会撤销授权并清除设备上的个人额度。',waiting:'等待连接设备',pairing:'正在连接设备…',preview:'预览模式：配对与同步暂未启用',pairSuccess:'配对成功，正在自动同步',pairFail:'配对未完成，请检查USB与设备确认',syncSuccess:'个人额度已同步',syncFail:'同步未完成：检查Wi-Fi、Mac登录与配对；设备保留旧记录',syncTimeout:'同步超时，设备保留旧记录，稍后重试',syncing:'自动同步已运行，等待设备连接',paused:'已暂停，设备会标注旧数据'},
'zh-TW':{title:'隨身星盤 · Mac 同步助手',languageLabel:'語言',flow:'已登入 Codex 的 Mac → 同一 Wi-Fi → 你的 AI Passport',setup:'首次使用 USB 傳輸線連接裝置，先點選下方「連接裝置」。裝置可能重新啟動並回到首頁；啟動後，長按硬體確定鍵進入設定，選擇「連接 Mac」，再於硬體按確定允許同步。配對成功後會自動同步，不必每次手動點選。',pair:'連接裝置',start:'恢復同步',pause:'暫停同步',quit:'結束助手',hint:'Mac 登入後，助手會自動啟動並恢復同步。Mac 需要保持執行；每 30 秒更新。離線或休眠時，裝置顯示舊額度和更新時間。可在系統設定→一般→登入項目中關閉自動啟動。登入憑證留在電腦；裝置只接收加密額度摘要。',revoke:'解除配對：在硬體設定中選擇「解除 Mac 配對」並確認。這會撤銷授權並清除裝置上的個人額度。',waiting:'等待連接裝置',pairing:'正在連接裝置…',preview:'預覽模式：尚未啟用配對與同步',pairSuccess:'配對成功，正在自動同步',pairFail:'配對未完成，請檢查 USB 和裝置確認',syncSuccess:'個人額度已同步',syncFail:'同步未完成：請檢查 Wi-Fi、Mac 登入狀態與配對；裝置會保留舊記錄',syncTimeout:'同步逾時，裝置會保留舊記錄，稍後再試',syncing:'自動同步已執行，等待裝置連線',paused:'已暫停，裝置會標示舊資料'},
en:{title:'Quota Orbit · Mac Sync Assistant',languageLabel:'Language',flow:'Signed-in Codex on your Mac → same Wi-Fi → your AI Passport',setup:'For first-time setup, connect the device with a USB data cable and click “Connect device”. The device may restart to its home screen. After it boots, hold Confirm on the device, open Settings > Connect Mac, then press Confirm to allow syncing. Sync starts automatically after pairing; no need to click Start each time.',pair:'Connect device',start:'Resume sync',pause:'Pause sync',quit:'Quit assistant',hint:'The assistant starts at Mac login and resumes syncing automatically. Keep your Mac running; data refreshes about every 30 seconds. When offline or asleep, the device shows the last value and update time. Disable auto-start in System Settings > General > Login Items. Sign-in credentials stay on your Mac; only an encrypted quota summary is sent to the device.',revoke:'To unpair, choose “Unpair Mac” in device Settings and confirm. This revokes access and clears personal quota data from the device.',waiting:'Waiting for device',pairing:'Connecting to device…',preview:'Preview mode: pairing and sync are disabled',pairSuccess:'Paired. Syncing automatically.',pairFail:'Pairing incomplete. Check USB and confirm on the device.',syncSuccess:'Personal quota synced',syncFail:'Sync incomplete. Check Wi-Fi, Mac sign-in and pairing; the device keeps the previous snapshot.',syncTimeout:'Sync timed out. The device keeps the previous snapshot; retrying later.',syncing:'Automatic sync is running; waiting for device',paused:'Sync paused. The device will mark the data as old.'}
};
let lang='zh-CN';try{const saved=localStorage.getItem('quotaOrbitLanguage');if(copy[saved])lang=saved}catch(_){}
function applyLanguage(){document.documentElement.lang=lang==='en'?'en':lang;document.title=copy[lang].title;document.querySelectorAll('[data-i18n]').forEach(e=>{const key=e.dataset.i18n;e.textContent=copy[lang][key]});document.querySelector('#language').value=lang;refresh()}
document.querySelector('#language').addEventListener('change',e=>{lang=copy[e.target.value]?e.target.value:'zh-CN';try{localStorage.setItem('quotaOrbitLanguage',lang)}catch(_){}applyLanguage()});
async function send(action){await fetch('/action',{method:'POST',headers:{'Content-Type':'application/json','X-Local-Token':'TOKEN'},body:JSON.stringify({action})});await refresh()}
async function refresh(){try{const r=await fetch('/status');const s=await r.json();const c=copy[lang];const suffix=s.updated_at?' · '+s.updated_at:'';document.querySelector('#status').textContent=(c[s.message]||c.waiting)+suffix;document.querySelectorAll('button').forEach(b=>{b.disabled=b.id!=='quit'&&(s.busy||!s.allow_device||(b.dataset.action==='start'&&s.syncing)||(b.dataset.action==='stop'&&!s.syncing))})}catch(_){document.querySelector('#status').textContent=copy[lang].waiting}}
document.querySelectorAll('button[data-action]').forEach(b=>b.addEventListener('click',()=>send(b.dataset.action)));applyLanguage();setInterval(refresh,1500);
</script></html>'''.replace('TOKEN',TOKEN)
def update(**values):
    with lock:state.update(values)
def start_sync_locked():
    global sync_thread
    if sync_thread is not None and sync_thread.is_alive():return False
    stop.clear();state['syncing']=True;state['message']='syncing'
    sync_thread=threading.Thread(target=sync_loop,daemon=True);sync_thread.start()
    return True
LOGIN_AGENT_LABEL='com.starshoreai.QuotaOrbitPassportHelper'
def install_login_agent(agent_dir=None, executable=None):
    """Register the bundled helper for the next macOS login, never in source/preview mode."""
    if sys.platform!='darwin' or not getattr(sys,'frozen',False):return False
    agent_dir=Path(agent_dir) if agent_dir else Path.home()/'Library'/'LaunchAgents'
    executable=str(executable or sys.executable)
    if not any(parent.suffix=='.app' for parent in Path(executable).parents):return False
    agent_dir.mkdir(parents=True,exist_ok=True,mode=0o700)
    try:agent_dir.chmod(0o700)
    except OSError:pass
    destination=agent_dir/(LOGIN_AGENT_LABEL+'.plist')
    payload={'Label':LOGIN_AGENT_LABEL,'ProgramArguments':[executable,'--allow-device','--no-browser','--auto-sync'],
             'RunAtLoad':True,'KeepAlive':False,'LimitLoadToSessionType':'Aqua','ProcessType':'Background'}
    fd,temp_path=tempfile.mkstemp(prefix=LOGIN_AGENT_LABEL+'.',suffix='.tmp',dir=agent_dir)
    try:
        with os.fdopen(fd,'wb') as stream:plistlib.dump(payload,stream,fmt=plistlib.FMT_XML,sort_keys=True)
        os.chmod(temp_path,0o600);os.replace(temp_path,destination)
    finally:
        try:os.unlink(temp_path)
        except FileNotFoundError:pass
    return destination

def bridge_args(name):
    return [sys.executable,'--bridge',name] if getattr(sys,'frozen',False) else [sys.executable,str(ROOT/'bridge.py'),name]
def spawn_bridge(name):
    process=subprocess.Popen(bridge_args(name),stdout=subprocess.PIPE,stderr=subprocess.DEVNULL,text=True,bufsize=1)
    with lock:children.add(process)
    return process
def command(name):
    process=spawn_bridge(name)
    for _line in process.stdout:
        # Bridge output is not a UI string; show a stable localized result instead.
        pass
    code=process.wait()
    with lock:children.discard(process)
    update(busy=False,message='pairSuccess' if name=='pair' and code==0 else 'pairFail' if name=='pair' else 'syncFail')
    return code
def pair_command(resume):
    thread=sync_thread
    if thread is not None and thread.is_alive():thread.join(timeout=55)
    code=command('pair')
    if resume:
        with lock:start_sync_locked()
def sync_loop():
    update(syncing=True)
    while not stop.is_set():
        try:
            process=spawn_bridge('once')
            try:
                process.communicate(timeout=50)
            except subprocess.TimeoutExpired:
                process.terminate();process.wait(timeout=3);raise
            finally:
                with lock:children.discard(process)
            update(message='syncSuccess' if process.returncode==0 else 'syncFail',updated_at=time.strftime('%H:%M:%S') if process.returncode==0 else '')
        except subprocess.TimeoutExpired:
            update(message='syncTimeout')
        if stop.wait(30):break
    update(syncing=False,message='paused')
class Handler(BaseHTTPRequestHandler):
    def log_message(self,*args):pass
    def valid_host(self):return self.headers.get('Host')==f'127.0.0.1:{PORT}'
    def reply(self,code,body,content='application/json'):
        self.send_response(code);self.send_header('Content-Type',content+'; charset=utf-8');self.send_header('Cache-Control','no-store');self.send_header('X-Content-Type-Options','nosniff');self.end_headers();self.wfile.write(body.encode())
    def do_GET(self):
        if not self.valid_host():self.reply(403,'{}');return
        if self.path=='/':self.reply(200,PAGE,'text/html')
        elif self.path=='/status':
            with lock:self.reply(200,json.dumps(state,ensure_ascii=False))
        else:self.reply(404,'{}')
    def do_POST(self):
        if not self.valid_host() or self.path!='/action' or self.headers.get('X-Local-Token')!=TOKEN or self.headers.get('Origin')!=f'http://127.0.0.1:{PORT}':self.reply(403,'{}');return
        length=int(self.headers.get('Content-Length','0'))
        if not 0<length<100:self.reply(400,'{}');return
        try:action=json.loads(self.rfile.read(length))['action']
        except (ValueError,KeyError):self.reply(400,'{}');return
        if action=='quit':
            self.reply(200,'{}');stop.set()
            threading.Thread(target=SERVER.shutdown,daemon=True).start();return
        with lock:
            if not state['allow_device'] or state['busy']:self.reply(409,'{}');return
            if action=='pair':
                resume=state['syncing'];state['busy']=True;state['message']='pairing'
                if resume:stop.set()
                threading.Thread(target=pair_command,args=(resume,),daemon=True).start()
            elif action=='start' and not state['syncing']:
                start_sync_locked()
            elif action=='stop':stop.set()
            else:self.reply(400,'{}');return
        self.reply(200,'{}')
def main():
    global PORT,SERVER,PAGE
    parser=argparse.ArgumentParser()
    parser.add_argument('--allow-device',action='store_true')
    parser.add_argument('--port',type=int,default=8942)
    parser.add_argument('--no-browser',action='store_true')
    parser.add_argument('--auto-sync',action='store_true')
    args=parser.parse_args()
    if not 1024<=args.port<=65535:parser.error('Invalid local port')
    PORT=args.port
    state['allow_device']=args.allow_device
    if not args.allow_device:state['message']='preview'
    if args.allow_device:
        try:install_login_agent()
        except OSError as error:print(f'无法配置登录启动：{error}',file=sys.stderr,flush=True)
    try:SERVER=ThreadingHTTPServer(('127.0.0.1',PORT),Handler)
    except OSError:
        if not args.no_browser:webbrowser.open(f'http://127.0.0.1:{PORT}/')
        raise SystemExit(0)
    if not args.no_browser:webbrowser.open(f'http://127.0.0.1:{PORT}/')
    print(f'Mac助手：http://127.0.0.1:{PORT}/（仅本机）',flush=True)
    if args.allow_device and args.auto_sync:
        with lock:start_sync_locked()
    try:SERVER.serve_forever()
    finally:
        stop.set();SERVER.server_close()
        with lock:running=list(children)
        for process in running:
            if process.poll() is None:
                process.terminate()
                try:process.wait(timeout=3)
                except subprocess.TimeoutExpired:process.kill();process.wait()
if __name__=='__main__':main()
