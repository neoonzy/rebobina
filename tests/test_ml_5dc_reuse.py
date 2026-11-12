"""Offline tests for licensed historical Magic Lantern code reuse."""
import importlib.util
import json
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SPEC = importlib.util.spec_from_file_location("ml_5dc_audit",
        ROOT / "tools/ml_5dc_audit.py")
mod = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(mod)


class HistoricalMLTests(unittest.TestCase):
    def test_audit_and_provenance(self):
        result = mod.audit()
        self.assertEqual(result["mode"], "OFFLINE_ONLY")
        self.assertEqual(result["rom_symbols"], 10)
        self.assertEqual(result["button_events"], 7)

    def test_no_binary_or_installation_claim(self):
        info = json.loads((mod.SOURCE / "source.json").read_text(encoding="utf-8"))
        self.assertIsNone(info["base_2012_ml_sha256"])
        self.assertFalse(info["hardware_verified"])
        self.assertFalse((mod.SOURCE / "AUTOEXEC.BIN").exists())
        self.assertFalse((mod.SOURCE / "REBOBINA.FIR").exists())


if __name__ == "__main__":
    unittest.main()
