import json, os, plistlib, stat, sys, tempfile, threading, unittest, urllib.request, urllib.error
from unittest.mock import patch
import assistant
import launcher
class AssistantTests(unittest.TestCase):
    def test_language_choices_are_present_and_persisted_locally(self):
        for value in ('zh-CN','zh-TW','en'):
            self.assertIn(f'<option value="{value}">',assistant.PAGE)
        self.assertIn("localStorage.setItem('quotaOrbitLanguage',lang)",assistant.PAGE)
        for message in ('pairing','pairSuccess','pairFail','syncSuccess','syncFail','syncTimeout','paused'):
            self.assertIn(message+':',assistant.PAGE)
        self.assertIn("document.documentElement.lang",assistant.PAGE)
    def test_frozen_worker_dispatch(self):
        with patch.object(sys,'frozen',True,create=True):
            self.assertEqual(assistant.bridge_args('once'),[sys.executable,'--bridge','once'])
        self.assertEqual(assistant.bridge_args('pair')[-2:],[str(assistant.ROOT/'bridge.py'),'pair'])
    def test_app_launch_defaults_to_auto_sync_but_preview_does_not(self):
        self.assertEqual(launcher.app_args(['app']),['app','--allow-device','--auto-sync'])
        self.assertEqual(launcher.app_args(['app','--preview']),['app'])
        self.assertEqual(launcher.app_args(['app','--allow-device']),['app','--allow-device','--auto-sync'])
    def test_login_agent_runs_quietly_and_auto_syncs(self):
        with tempfile.TemporaryDirectory() as directory, \
             patch.object(assistant.sys,'platform','darwin'), \
             patch.object(assistant.sys,'frozen',True,create=True):
            executable='/Applications/StarshorePassport.app/Contents/MacOS/StarshorePassport'
            path=assistant.install_login_agent(directory,executable)
            self.assertEqual(path.name,assistant.LOGIN_AGENT_LABEL+'.plist')
            with path.open('rb') as stream:payload=plistlib.load(stream)
            self.assertEqual(payload['ProgramArguments'],[executable,'--allow-device','--no-browser','--auto-sync'])
            self.assertTrue(payload['RunAtLoad'])
            self.assertFalse(payload['KeepAlive'])
            self.assertEqual(stat.S_IMODE(path.stat().st_mode),0o600)
            self.assertEqual(stat.S_IMODE(os.stat(directory).st_mode),0o700)
        self.assertIn('Mac登录后助手会自动启动',assistant.PAGE)
        self.assertIn("start:'恢复同步'",assistant.PAGE)
        self.assertIn("action=='pair'",(assistant.ROOT/'assistant.py').read_text(encoding='utf-8'))
    def test_device_enabled_launch_starts_sync_worker(self):
        previous_thread=assistant.sync_thread
        started=threading.Event();release=threading.Event()
        def fake_sync_loop():started.set();release.wait(1)
        try:
            with patch.object(assistant,'sync_loop',side_effect=fake_sync_loop):
                with assistant.lock:self.assertTrue(assistant.start_sync_locked())
                self.assertTrue(started.wait(1))
                with assistant.lock:
                    self.assertFalse(assistant.start_sync_locked())
                    self.assertTrue(assistant.state['syncing'])
        finally:
            assistant.stop.set()
            release.set()
            if assistant.sync_thread:assistant.sync_thread.join(timeout=1)
            with assistant.lock:
                assistant.sync_thread=previous_thread;assistant.state['syncing']=False;assistant.state['message']='waiting'
            assistant.stop.clear()
    def test_pairing_temporarily_pauses_auto_sync(self):
        server=assistant.ThreadingHTTPServer(('127.0.0.1',0),assistant.Handler)
        previous_port,previous_server=assistant.PORT,assistant.SERVER
        previous_state=dict(assistant.state);assistant.PORT=server.server_port;assistant.SERVER=server
        assistant.state.update(allow_device=True,busy=False,syncing=True,message='syncing')
        assistant.stop.clear();started=threading.Event()
        thread=threading.Thread(target=server.serve_forever,daemon=True);thread.start()
        url=f'http://127.0.0.1:{server.server_port}'
        request=urllib.request.Request(url+'/action',data=b'{"action":"pair"}',headers={
            'Origin':url,'X-Local-Token':assistant.TOKEN,'Content-Type':'application/json'})
        try:
            with patch.object(assistant,'pair_command',side_effect=lambda resume:started.set()):
                with urllib.request.urlopen(request) as response:self.assertEqual(response.status,200)
                self.assertTrue(started.wait(1));self.assertTrue(assistant.stop.is_set())
        finally:
            server.shutdown();thread.join(timeout=2);server.server_close()
            assistant.PORT=previous_port;assistant.SERVER=previous_server
            assistant.state.clear();assistant.state.update(previous_state);assistant.stop.clear()
    def test_preview_loopback_and_quit(self):
        server=assistant.ThreadingHTTPServer(('127.0.0.1',0),assistant.Handler)
        previous_port,previous_server=assistant.PORT,assistant.SERVER
        assistant.PORT=server.server_port;assistant.SERVER=server
        assistant.state['allow_device']=False
        thread=threading.Thread(target=server.serve_forever,daemon=True);thread.start()
        url=f'http://127.0.0.1:{server.server_port}'
        opener=urllib.request.build_opener(urllib.request.ProxyHandler({}))
        try:
            with opener.open(url+'/status') as r:self.assertFalse(json.load(r)['allow_device'])
            request=urllib.request.Request(url+'/status',headers={'Host':f'localhost:{server.server_port}'})
            with self.assertRaises(urllib.error.HTTPError) as error:opener.open(request)
            self.assertEqual(error.exception.code,403)
            request=urllib.request.Request(url+'/action',data=b'{"action":"pair"}',headers={'Origin':url})
            with self.assertRaises(urllib.error.HTTPError) as error:opener.open(request)
            self.assertEqual(error.exception.code,403)
            headers={'Origin':url,'X-Local-Token':assistant.TOKEN}
            request=urllib.request.Request(url+'/action',data=b'{"action":"pair"}',headers=headers)
            with self.assertRaises(urllib.error.HTTPError) as error:opener.open(request)
            self.assertEqual(error.exception.code,409)
            request=urllib.request.Request(url+'/action',data=b'{"action":"quit"}',headers=headers)
            with opener.open(request) as r:self.assertEqual(r.status,200)
            thread.join(timeout=2);self.assertFalse(thread.is_alive())
        finally:
            if thread.is_alive():server.shutdown()
            server.server_close();assistant.PORT=previous_port;assistant.SERVER=previous_server
            assistant.stop.clear()
if __name__=='__main__':unittest.main()
