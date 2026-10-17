"""Build an isolated ARM64 helper candidate, then audit its complete dependency set."""
import argparse
import importlib.metadata
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
import sysconfig
from macos_bundle import audit_bundle, parse_version


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', required=True, type=Path)
    parser.add_argument('--version', required=True)
    parser.add_argument('--minimum-macos', default='11.0')
    parser.add_argument('--sign-identity', help='Optional Developer ID identity; otherwise use ad-hoc signing')
    args = parser.parse_args()
    if sys.platform != 'darwin':
        parser.error('build on macOS with an isolated compatible Python environment')
    parse_version(args.minimum_macos)
    if not all(part.isdigit() for part in args.version.split('.')) or len(args.version.split('.')) != 3:
        parser.error('version must have three numeric components')
    output = args.output.resolve()
    if output.exists():
        parser.error('output must be a new directory; previous builds are never overwritten')
    output.mkdir(parents=True)
    source = Path(__file__).resolve().parent
    spec = output / 'StarshorePassport.spec'
    spec.write_text(f'''a = Analysis([{str(source / 'launcher.py')!r}], pathex=[{str(source)!r}],
    binaries=[], datas=[], hiddenimports=[], hookspath=[], hooksconfig={{}},
    runtime_hooks=[], excludes=[], noarchive=False, optimize=0)
pyz = PYZ(a.pure)
exe = EXE(pyz, a.scripts, [], exclude_binaries=True, name='StarshorePassport',
    debug=False, bootloader_ignore_signals=False, strip=False, upx=False,
    console=False, disable_windowed_traceback=False, argv_emulation=False,
    target_arch='arm64', codesign_identity={args.sign_identity!r}, entitlements_file=None)
coll = COLLECT(exe, a.binaries, a.datas, strip=False, upx=False, name='StarshorePassport')
app = BUNDLE(coll, name='StarshorePassport.app', bundle_identifier='com.starshoreai.passport-helper',
    version={args.version!r}, info_plist={{'LSMinimumSystemVersion': {args.minimum_macos!r},
    'CFBundleShortVersionString': {args.version!r}}})
''')
    env = dict(os.environ, PYINSTALLER_CONFIG_DIR=str(output / 'cache'),
               MACOSX_DEPLOYMENT_TARGET=args.minimum_macos)
    subprocess.run([sys.executable, '-m', 'PyInstaller', '--distpath', str(output / 'dist'),
                    '--workpath', str(output / 'work'), str(spec)], check=True, env=env)
    app = output / 'dist/StarshorePassport.app'
    notices = app / 'Contents/Resources/ThirdPartyNotices'
    notices.mkdir()
    shutil.copyfile(source / 'LICENSE', notices / 'Passport-AGPL-3.0.txt')
    packages = []
    for dist in importlib.metadata.distributions():
        name = dist.metadata['Name']
        packages.append({'name': name, 'version': dist.version})
        for file in dist.files or []:
            # Keep original license text and relative hierarchy to avoid name collisions.
            if any('license' in part.lower() or 'licence' in part.lower() or
                   part.lower() in ('copying', 'notice', 'notices') for part in file.parts):
                origin = Path(dist.locate_file(file))
                if not origin.is_file():
                    continue
                relative = Path(*file.parts)
                if relative.is_absolute() or '..' in relative.parts:
                    continue
                destination = notices / name / relative
                destination.parent.mkdir(parents=True, exist_ok=True)
                shutil.copyfile(origin, destination)
    shutil.copyfile(Path(sysconfig.get_path('stdlib')) / 'LICENSE.txt', notices / 'Python-LICENSE.txt')
    (notices / 'dependencies.json').write_text(json.dumps(sorted(packages, key=lambda p: p['name'].lower()), indent=2) + '\n')
    # Resource additions invalidate the outer signature; re-sign only this new bundle.
    identity = args.sign_identity or '-'
    command = ['/usr/bin/codesign', '--force', '--sign', identity]
    if args.sign_identity:
        command += ['--options', 'runtime', '--timestamp']
    subprocess.run(command + [str(app)], check=True)
    subprocess.run(['/usr/bin/codesign', '--verify', '--deep', '--strict', str(app)], check=True)
    audit = audit_bundle(app, args.minimum_macos)
    (output / 'bundle-audit.json').write_text(json.dumps(audit, indent=2) + '\n')
    if not audit['passed']:
        raise SystemExit('Bundle audit failed: ' + '; '.join(audit['errors']))
    manifest = {'version': args.version, 'minimum_macos_target': args.minimum_macos,
                'architecture': 'arm64', 'python': sys.version.split()[0],
                'packages': sorted(packages, key=lambda p: p['name'].lower()),
                'macho_count': len(audit['binaries']), 'older_os_runtime_verified': False,
                'source_commit': subprocess.check_output(['git', '-C', str(source), 'rev-parse', 'HEAD'], text=True).strip()}
    (output / 'build-manifest.json').write_text(json.dumps(manifest, indent=2) + '\n')
    print(f'Candidate ready: {app}\nMinimum-OS audit: PASS; older-OS runtime: NOT VERIFIED')


if __name__ == '__main__':
    main()
