"""Regression tests for minimum-OS and host-library leakage in helper bundles."""
import plistlib
import struct
import tempfile
import unittest
from pathlib import Path
from macos_bundle import ARM64, allowed_dependency, audit_bundle, macho_slices


def thin(minimum=(11, 0, 0), cpu=ARM64, dependency=None, sdk=(27, 0, 0)):
    packed = lambda v: (v[0] << 16) | (v[1] << 8) | v[2]
    commands = struct.pack('<IIIIII', 0x32, 24, 1, packed(minimum), packed(sdk), 0)
    if dependency:
        name = dependency.encode() + b'\0'
        size = (24 + len(name) + 7) & ~7
        commands += struct.pack('<IIIIII', 0xC, size, 24, 0, 0, 0) + name + bytes(size - 24 - len(name))
    return struct.pack('<IIIIIIII', 0xFEEDFACF, cpu, 0, 2, 1 + bool(dependency), len(commands), 0, 0) + commands


class MacBundleTests(unittest.TestCase):
    def bundle(self, data, declared='11.0'):
        directory = tempfile.TemporaryDirectory()
        self.addCleanup(directory.cleanup)
        root = Path(directory.name)
        exe = root / 'Contents/MacOS/StarshorePassport'
        exe.parent.mkdir(parents=True)
        exe.write_bytes(data)
        (root / 'Contents/Info.plist').write_bytes(plistlib.dumps({'LSMinimumSystemVersion': declared}))
        return root

    def test_new_sdk_does_not_raise_minimum(self):
        self.assertTrue(audit_bundle(self.bundle(thin()))['passed'])

    def test_transitive_library_minimum_is_checked(self):
        root = self.bundle(thin())
        lib = root / 'Contents/Frameworks/liblzma.dylib'
        lib.parent.mkdir()
        lib.write_bytes(thin((27, 0, 0)))
        self.assertFalse(audit_bundle(root)['passed'])

    def test_homebrew_dependency_is_rejected(self):
        result = audit_bundle(self.bundle(thin(dependency='/opt/homebrew/lib/libssl.3.dylib')))
        self.assertTrue(any('external dependency' in error for error in result['errors']))

    def test_system_dependency_is_allowed(self):
        self.assertTrue(audit_bundle(self.bundle(thin(dependency='/usr/lib/libSystem.B.dylib')))['passed'])

    def test_loader_directory_rpath_is_allowed(self):
        self.assertTrue(allowed_dependency('@loader_path'))
        self.assertFalse(allowed_dependency('/opt/homebrew/lib'))

    def test_wrong_architecture_is_rejected(self):
        self.assertFalse(audit_bundle(self.bundle(thin(cpu=0x1000007)))['passed'])

    def test_plist_cannot_claim_a_different_floor(self):
        self.assertFalse(audit_bundle(self.bundle(thin(), '10.13'))['passed'])

    def test_universal_arm64_slice_is_selected(self):
        intel, arm = thin((10, 13, 0), cpu=0x1000007), thin()
        header = struct.pack('>II', 0xCAFEBABE, 2)
        header += struct.pack('>IIIII', 0x1000007, 0, 48, len(intel), 0)
        header += struct.pack('>IIIII', ARM64, 0, 48 + len(intel), len(arm), 0)
        self.assertTrue(audit_bundle(self.bundle(header + intel + arm))['passed'])

    def test_truncated_commands_fail_closed(self):
        with self.assertRaises(ValueError):
            macho_slices(thin()[:-1])

    def test_escaping_symlink_is_rejected(self):
        root = self.bundle(thin())
        (root / 'external').symlink_to('/usr/lib')
        self.assertFalse(audit_bundle(root)['passed'])


if __name__ == '__main__':
    unittest.main()
