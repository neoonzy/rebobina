"""Offline regression tests for 5DC historical boot-chain provenance."""
import copy
import importlib.util
import json
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SPEC = importlib.util.spec_from_file_location("audit_5dc_boot_chain",
                                             ROOT / "tools/audit_5dc_boot_chain.py")
module = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(module)


class BootChainAuditTests(unittest.TestCase):
    def setUp(self):
        self.data = json.loads(module.DATA.read_text(encoding="utf-8"))

    def test_pinned_data_and_header(self):
        self.assertTrue(module.audit())

    def test_reject_reversed_load_addresses(self):
        data = copy.deepcopy(self.data)
        data["historical_address_roles"][0]["value"] = "0x00980000"
        with self.assertRaises(ValueError):
            module.validate_data(data)

    def test_reject_installing_historical_toggle(self):
        data = copy.deepcopy(self.data)
        data["source"]["installer_include_in_top_level_build"] = True
        with self.assertRaises(ValueError):
            module.validate_data(data)

    def test_reject_hardware_claim_without_evidence(self):
        data = copy.deepcopy(self.data)
        data["camera_installation_authorized"] = True
        with self.assertRaises(ValueError):
            module.validate_data(data)

    def test_reject_changed_signature_and_source(self):
        data = copy.deepcopy(self.data)
        data["bootflags"]["write_opcode"] = "0xE92D0000"
        with self.assertRaises(ValueError):
            module.validate_data(data)
        data = copy.deepcopy(self.data)
        data["source"]["commit"] = "master"
        with self.assertRaises(ValueError):
            module.validate_data(data)


if __name__ == "__main__":
    unittest.main()
