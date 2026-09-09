#!/usr/bin/env python3
"""Check fail-closed Binder readiness and the recovery HAL startup ordering."""
from pathlib import Path
import os
import subprocess
import tempfile
import unittest

TREE = Path(__file__).resolve().parents[1]


class ReadinessTests(unittest.TestCase):
    def test_binder_check_requires_successful_checker(self):
        source = (TREE / 'recovery/root/system/bin/pbrp-decrypt-prereqs').read_text()
        function = source.split('binder_service_available() {', 1)[1].split('\nwait_binder_service()', 1)[0]
        function = 'binder_service_available() {' + function
        with tempfile.TemporaryDirectory() as name:
            checker = Path(name) / 'checker'
            script = function.replace('/system/bin/pbrp-service-check', str(checker))
            script += '\nbinder_service_available android.hardware.weaver.IWeaver/default\n'
            def run():
                return subprocess.run(['/bin/sh', '-c', script], capture_output=True).returncode
            self.assertNotEqual(run(), 0, 'missing checker must not imply readiness')
            checker.write_text('#!/bin/sh\necho "Service not found"\nexit 1\n')
            checker.chmod(0o755)
            self.assertNotEqual(run(), 0, 'an absent service must not pass substring matching')
            checker.write_text('#!/bin/sh\nexit 0\n')
            self.assertEqual(run(), 0)

    def test_gate_rejects_unrestored_vendor_before_mutations(self):
        source = (TREE / 'recovery/root/system/bin/pbrp-decrypt-prereqs').read_text()
        # Isolate the entry checks before mounting persist or touching any HAL.
        entry = source.split('setprop twrp.decrypt.prereq preparing', 1)[1].split('\nmount_persist', 1)[0]
        with tempfile.TemporaryDirectory() as name:
            checker = Path(name) / 'checker'
            checker.write_text('#!/bin/sh\nexit 0\n')
            checker.chmod(0o755)
            for state, succeeds in [('0', False), ('', False), ('1', True)]:
                script = '''setprop() { :; }
set_status() { :; }
fail_prereq() { exit 1; }
getprop() { printf '%s' "$FIXTURE_READY"; }
setprop twrp.decrypt.prereq preparing'''+entry.replace('/system/bin/pbrp-service-check', str(checker))
                result = subprocess.run(['/bin/sh', '-c', script], env=dict(os.environ, FIXTURE_READY=state), capture_output=True)
                self.assertEqual(result.returncode == 0, succeeds, result.stderr)

    def test_early_init_cannot_start_deferred_hals(self):
        root = TREE / 'recovery/root'
        names = {
            'vendor.secure_element', 'vendor.weaver_nxp', 'vendor.keymint-strongbox',
            'vendor.touch-aidl-2', 'vendor.health-default',
        }
        definitions = {}
        for p in root.rglob('*.rc'):
            current = None
            for line in p.read_text().splitlines():
                if line.startswith('service '):
                    current = line.split()[1]
                    if current in names:
                        definitions[current] = []
                elif line and not line[0].isspace() and not line.startswith('#'):
                    current = None
                elif current in names:
                    definitions[current].append(line.strip())
        self.assertEqual(set(definitions), names)
        for name, options in definitions.items():
            self.assertIn('disabled', options, name)
        init = (root / 'init.recovery.qcom.rc').read_text()
        active = ''
        for line in init.splitlines():
            if line.startswith('on '):
                active = line
            if line.strip() in ('start pbrp-touch-start', 'start pbrp-decrypt-prereqs', 'start vendor.health-default'):
                self.assertIn('property:twrp.fstab.ready=1', active)

    def test_android_16_hals_use_private_wrapper(self):
        root = TREE / 'recovery/root'
        required = {
            'vendor.boot-qti', 'vendor.secure_element', 'vendor.weaver_nxp',
            'vendor.keymint-strongbox', 'se_omapi',
        }
        commands = {}
        for path in root.rglob('*.rc'):
            for line in path.read_text().splitlines():
                if line.startswith('service '):
                    fields = line.split()
                    if fields[1] in required:
                        commands[fields[1]] = fields[2:]
        self.assertEqual(set(commands), required)
        for name, command in commands.items():
            self.assertEqual(command[0], '/system/bin/pbrp-a16-wrap', name)

    def test_aidl_boot_control_preserves_legacy_callers(self):
        board = (TREE / 'BoardConfig.mk').read_text()
        self.assertIn('TW_USE_AIDL_BOOT_CONTROL := true', board)
        self.assertIn('android.hardware.boot@1.2-impl.recovery', board)
        wrapper = (TREE / 'recovery/root/system/bin/pbrp-bootctl').read_text()
        self.assertIn('/system/bin/pbrp-a16-wrap /system/bin/pbrp-bootctl-a16 "$@"', wrapper)
        for path in (
            TREE / 'recovery/root/system/bin/pbrp-bootctl-a16',
            TREE / 'recovery/root/system/lib64/pbrp-a16-compat/libboot_control_client.so',
        ):
            self.assertTrue(path.read_bytes().startswith(b'\x7fELF'), path)


if __name__ == '__main__':
    unittest.main(verbosity=2)
