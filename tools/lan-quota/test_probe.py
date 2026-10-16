import json, subprocess, sys, unittest
from unittest.mock import patch
import probe
class ProbeTests(unittest.TestCase):
    def test_login_environment_includes_node_paths_without_mutating_parent(self):
        with patch.dict('probe.os.environ', {'PATH':'/usr/bin:/bin'}, clear=True):
            env=probe.codex_environment()
            self.assertEqual(env['PATH'], '/usr/bin:/bin:/opt/homebrew/bin:/usr/local/bin')
            self.assertEqual(probe.os.environ['PATH'], '/usr/bin:/bin')

    def test_login_path_finds_homebrew_codex(self):
        with patch('probe.shutil.which', return_value=None), \
             patch('probe.Path.is_file', lambda p: str(p) == '/opt/homebrew/bin/codex'), \
             patch('probe.os.access', return_value=True):
            self.assertEqual(probe.codex_executable(), '/opt/homebrew/bin/codex')
    def test_nonexecutable_codex_is_not_selected(self):
        with patch('probe.shutil.which', return_value=None), \
             patch('probe.Path.is_file', return_value=True), \
             patch('probe.os.access', return_value=False):
            with self.assertRaises(FileNotFoundError): probe.codex_executable()
    def test_shell_path_keeps_precedence(self):
        with patch('probe.shutil.which', return_value='/custom/bin/codex'), \
             patch('probe.Path.is_file', return_value=True), \
             patch('probe.os.access', return_value=True):
            self.assertEqual(probe.codex_executable(), '/custom/bin/codex')

    def test_bucket_selection_and_missing(self):
        window={'usedPercent':25,'windowDurationMins':10080,'resetsAt':1800000000}
        other={'limitId':'codex_other','primary':window}
        self.assertEqual(probe.windows_from_result({'rateLimits':other}),[])
        self.assertEqual(probe.windows_from_result({'rateLimits':{'limitId':'codex','primary':window},'rateLimitsByLimitId':{'codex_other':other}}),[])
        windows=probe.windows_from_result({'rateLimitsByLimitId':{'codex':{'secondary':window}}})
        self.assertEqual(len(windows),1);self.assertEqual(windows[0]['remainingPercent'],75)
        self.assertEqual(probe.windows_from_result({'rateLimits':{'primary':{'usedPercent':True}}}),[])
        self.assertEqual(probe.windows_from_result({'rateLimits':{'primary':{'usedPercent':float('nan')}}}),[])
    def test_batched_notification_and_response(self):
        script="""import sys,json,os
for line in sys.stdin:
 request=json.loads(line)
 if request.get('id')==1: print(json.dumps({'id':1,'result':{}}),flush=True)
 if request.get('id')==2:
  result={'rateLimits':{'limitId':'codex','secondary':{'usedPercent':42,'windowDurationMins':10080,'resetsAt':1800000000}}}
  os.write(1,(json.dumps({'method':'account/rateLimits/updated','params':{}})+'\\n'+json.dumps({'id':2,'result':result})+'\\n').encode())
"""
        original=subprocess.Popen
        def factory(*args,**kwargs):return original([sys.executable,'-u','-c',script],**kwargs)
        with patch('probe.subprocess.Popen',side_effect=factory):windows=probe.read_quota(timeout=2)
        self.assertEqual(windows[0]['remainingPercent'],58)
if __name__=='__main__':unittest.main()
