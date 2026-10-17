"""Audit every bundled ARM64 Mach-O before distributing the Mac helper."""
import argparse
import json
import plistlib
import struct
from pathlib import Path

ARM64 = 0x100000C
MAGICS = {b'\xcf\xfa\xed\xfe': ('<', 32), b'\xfe\xed\xfa\xcf': ('>', 32),
          b'\xce\xfa\xed\xfe': ('<', 28), b'\xfe\xed\xfa\xce': ('>', 28)}
FAT_MAGICS = {b'\xca\xfe\xba\xbe': ('>', 20), b'\xbe\xba\xfe\xca': ('<', 20),
              b'\xca\xfe\xba\xbf': ('>', 32), b'\xbf\xba\xfe\xca': ('<', 32)}


def version(value):
    return (value >> 16, (value >> 8) & 255, value & 255)


def parse_version(value):
    parts = tuple(int(part) for part in value.split('.'))
    if not 1 <= len(parts) <= 3 or any(part < 0 or part > 255 for part in parts):
        raise ValueError('invalid macOS version')
    return parts + (0,) * (3 - len(parts))


def macho_slices(data):
    """Read load commands, including universal binaries; reject truncated inputs."""
    magic = data[:4]
    if magic in FAT_MAGICS:
        endian, size = FAT_MAGICS[magic]
        if len(data) < 8:
            raise ValueError('truncated fat header')
        count = struct.unpack_from(endian + 'I', data, 4)[0]
        if not 1 <= count <= 32 or 8 + count * size > len(data):
            raise ValueError('invalid fat architecture table')
        result = []
        for index in range(count):
            pos = 8 + index * size
            offset, length = struct.unpack_from(endian + ('QQ' if size == 32 else 'II'), data, pos + 8)
            if offset < 8 + count * size or length < 28 or offset + length > len(data):
                raise ValueError('invalid fat slice bounds')
            slices = macho_slices(data[offset:offset + length])
            if len(slices) != 1:
                raise ValueError('invalid nested Mach-O slice')
            result.extend(slices)
        return result
    if magic not in MAGICS:
        return []
    endian, header = MAGICS[magic]
    if len(data) < header:
        raise ValueError('truncated Mach-O header')
    cpu = struct.unpack_from(endian + 'I', data, 4)[0]
    count, command_bytes = struct.unpack_from(endian + 'II', data, 16)
    end = header + command_bytes
    if end > len(data) or count > command_bytes // 8:
        raise ValueError('invalid load command bounds')
    result = {'cpu': cpu, 'minimum': None, 'platform': None, 'dependencies': [], 'rpaths': []}
    pos = header
    for _ in range(count):
        if pos + 8 > end:
            raise ValueError('truncated load command')
        cmd, size = struct.unpack_from(endian + 'II', data, pos)
        if size < 8 or pos + size > end:
            raise ValueError('invalid load command size')
        if cmd == 0x32:  # LC_BUILD_VERSION; do not mistake SDK for minimum OS.
            if size < 24:
                raise ValueError('truncated build version')
            result['platform'], minimum = struct.unpack_from(endian + 'II', data, pos + 8)
            result['minimum'] = version(minimum)
        elif cmd == 0x24:  # LC_VERSION_MIN_MACOSX
            if size < 16:
                raise ValueError('truncated minimum version')
            result['platform'] = 1
            result['minimum'] = version(struct.unpack_from(endian + 'I', data, pos + 8)[0])
        elif cmd in (0xC, 0x80000018, 0x8000001F, 0x80000023, 0x20, 0x8000001C):
            if size < 12:
                raise ValueError('truncated dependency command')
            offset = struct.unpack_from(endian + 'I', data, pos + 8)[0]
            if not 12 <= offset < size:
                raise ValueError('invalid dependency name offset')
            raw = data[pos + offset:pos + size]
            if b'\0' not in raw:
                raise ValueError('unterminated dependency name')
            name = raw.split(b'\0', 1)[0].decode('utf-8')
            result['rpaths' if cmd == 0x8000001C else 'dependencies'].append(name)
        pos += size
    if pos != end:
        raise ValueError('inconsistent load command count')
    return [result]


def allowed_dependency(name):
    return name in ('@loader_path', '@executable_path', '/usr/lib', '/System/Library') or name.startswith(('@loader_path/', '@executable_path/', '@rpath/',
                            '/usr/lib/', '/System/Library/'))


def audit_bundle(bundle, target='11.0'):
    root = Path(bundle).resolve()
    limit = parse_version(target)
    errors, binaries = [], []
    try:
        info = plistlib.loads((root / 'Contents/Info.plist').read_bytes())
        declared = parse_version(info.get('LSMinimumSystemVersion', ''))
        if declared != limit:
            errors.append('Info.plist minimum must equal the audited target')
    except (OSError, ValueError, plistlib.InvalidFileException):
        errors.append('missing or invalid Info.plist minimum')
    for path in sorted(root.rglob('*')):
        relative = str(path.relative_to(root))
        if path.is_symlink():
            if not path.resolve().is_relative_to(root) or not path.exists():
                errors.append(f'{relative}: broken or escaping symlink')
            continue
        if not path.is_file():
            continue
        with path.open('rb') as stream:
            if stream.read(4) not in MAGICS.keys() | FAT_MAGICS.keys():
                continue
        try:
            slices = macho_slices(path.read_bytes())
        except (ValueError, struct.error, UnicodeDecodeError) as exc:
            errors.append(f'{relative}: {exc}')
            continue
        arm = [item for item in slices if item['cpu'] == ARM64]
        if len(arm) != 1:
            errors.append(f'{relative}: expected one ARM64 slice')
            continue
        item = arm[0]
        if item['platform'] != 1 or item['minimum'] is None or item['minimum'] > limit:
            errors.append(f'{relative}: minimum macOS {item["minimum"]} exceeds target or is unknown')
        for name in item['dependencies'] + item['rpaths']:
            if not allowed_dependency(name):
                errors.append(f'{relative}: external dependency/rpath {name}')
        binaries.append({'path': relative, **item})
    if not binaries:
        errors.append('bundle contains no ARM64 Mach-O binaries')
    if not any(item['path'].startswith('Contents/MacOS/') for item in binaries):
        errors.append('bundle has no audited executable')
    return {'target': list(limit), 'binaries': binaries, 'errors': errors, 'passed': not errors}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('bundle', type=Path)
    parser.add_argument('--target', default='11.0')
    parser.add_argument('--report', type=Path)
    args = parser.parse_args()
    result = audit_bundle(args.bundle, args.target)
    output = json.dumps(result, indent=2) + '\n'
    if args.report:
        args.report.write_text(output)
    print(f'Mac bundle audit: {"PASS" if result["passed"] else "FAIL"} ({len(result["binaries"])} binaries)')
    for error in result['errors']:
        print(error)
    raise SystemExit(0 if result['passed'] else 1)


if __name__ == '__main__':
    main()
