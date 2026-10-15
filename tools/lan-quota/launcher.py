"""Frozen app entry: dispatch workers without starting another local server."""
import sys
import bridge
import assistant
def app_args(argv):
    if '--preview' in argv:
        argv.remove('--preview')
    else:
        if '--allow-device' not in argv:argv.append('--allow-device')
        if '--auto-sync' not in argv:argv.append('--auto-sync')
    return argv
if __name__ == '__main__':
    if len(sys.argv)>1 and sys.argv[1]=='--bridge':
        del sys.argv[1]
        bridge.main()
    else:
        app_args(sys.argv)
        assistant.main()
