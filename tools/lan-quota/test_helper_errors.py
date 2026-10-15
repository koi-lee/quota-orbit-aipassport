import errno, io, json, tempfile, unittest
from pathlib import Path
from unittest.mock import patch
import assistant
import bridge

class ErrorTests(unittest.TestCase):
    def test_missing_and_invalid_pairing_do_not_probe_network(self):
        with tempfile.TemporaryDirectory() as directory:
            state=Path(directory)/'pair.json'
            with patch.object(bridge,'STATE',state),patch.object(bridge,'select_device') as select:
                with self.assertRaises(bridge.SyncFailure) as error:bridge.push_once()
                self.assertEqual(assistant.sync_message(error.exception.code),'pairMissing')
                state.write_text('{invalid')
                with self.assertRaises(bridge.SyncFailure) as error:bridge.push_once()
                self.assertEqual(assistant.sync_message(error.exception.code),'pairInvalid')
                select.assert_not_called()
    def test_network_and_codex_failures_are_distinct_and_private(self):
        with tempfile.TemporaryDirectory() as directory:
            state=Path(directory)/'pair.json';state.write_text(json.dumps({'key':'00'*32,'ip':'192.168.1.2','serial':'test'}))
            with patch.object(bridge,'STATE',state),patch.object(bridge,'select_device',side_effect=OSError('private details')):
                with self.assertRaises(bridge.SyncFailure) as error:bridge.push_once()
                self.assertEqual(assistant.sync_message(error.exception.code),'deviceUnavailable')
                self.assertNotIn('private details',str(error.exception))
            with patch.object(bridge,'STATE',state),patch.object(bridge,'select_device',return_value='192.168.1.2'),patch.object(bridge,'read_quota',side_effect=RuntimeError('private account')),patch.object(bridge,'push_to_device') as push:
                with self.assertRaises(bridge.SyncFailure) as error:bridge.push_once()
                self.assertEqual(assistant.sync_message(error.exception.code),'codexUnavailable')
                self.assertNotIn('private account',str(error.exception));push.assert_not_called()
    def test_unknown_exit_does_not_claim_login_failure(self):
        self.assertEqual(assistant.sync_message(1),'syncFail')
        self.assertEqual(assistant.sync_message(0),'syncSuccess')
    def test_occupied_port_does_not_change_login_agent_or_open_other_service(self):
        with patch('sys.argv',['helper','--allow-device','--auto-sync']),patch.object(assistant,'ThreadingHTTPServer',side_effect=OSError(errno.EADDRINUSE,'occupied')),patch.object(assistant,'existing_helper',return_value=False),patch.object(assistant,'show_port_conflict') as notice,patch.object(assistant,'install_login_agent') as login,patch.object(assistant.webbrowser,'open') as browser:
            with self.assertRaises(SystemExit) as result:assistant.main()
            self.assertEqual(result.exception.code,1);login.assert_not_called();browser.assert_not_called();notice.assert_called_once_with(8942,True)
    def test_duplicate_launch_opens_only_recognized_helper(self):
        with patch('sys.argv',['helper','--allow-device']),patch.object(assistant,'ThreadingHTTPServer',side_effect=OSError(errno.EADDRINUSE,'occupied')),patch.object(assistant,'existing_helper',return_value=True),patch.object(assistant,'install_login_agent') as login,patch.object(assistant.webbrowser,'open') as browser:
            with self.assertRaises(SystemExit) as result:assistant.main()
            self.assertEqual(result.exception.code,0);login.assert_not_called();browser.assert_called_once_with('http://127.0.0.1:8942/')
    def test_existing_helper_recognizes_legacy_and_rejects_other_page(self):
        for page,expected in [(assistant.PAGE,True),('<html>unrelated</html>',False)]:
            opener=unittest.mock.Mock();opener.open.return_value=io.BytesIO(page.encode())
            with patch.object(assistant.urllib.request,'build_opener',return_value=opener):self.assertEqual(assistant.existing_helper(8942),expected)

if __name__=='__main__':unittest.main()
