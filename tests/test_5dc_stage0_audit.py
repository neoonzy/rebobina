"""Negative tests for offline-only 5DC bootstrap inspection."""
import importlib.util
import tempfile
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SPEC = importlib.util.spec_from_file_location("stage0_audit",
    ROOT / "tools/audit_5dc_stage0.py")
mod = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(mod)


class BootstrapAuditTests(unittest.TestCase):
    def test_no_installable_output_sources(self):
        boot = ROOT / "camera/5DC.111/bootstrap"
        self.assertFalse(any(boot.glob("AUTOEXEC.BIN")))
        self.assertFalse(any(boot.glob("*.FIR")))
        self.assertFalse(any(boot.glob("*.fir")))
        self.assertIn("OFFLINE", (boot / "stage0_entry.S").read_text())
        self.assertIn("NEVER", (boot / "stage0_offline.ld").read_text())

    def test_reject_wrong_file_type(self):
        with tempfile.TemporaryDirectory() as d:
            bad = Path(d) / "malware.BIN"
            bad.write_bytes(b"gaonisoy")
            with self.assertRaises(ValueError):
                mod.audit(bad)

    def test_reject_fake_elf(self):
        with tempfile.TemporaryDirectory() as d:
            bad = Path(d) / "stage0-offline.elf"
            bad.write_bytes(b"not an ELF")
            with self.assertRaises(ValueError):
                mod.audit(bad)


if __name__ == "__main__":
    unittest.main()
