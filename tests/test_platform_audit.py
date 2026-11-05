"""Testes offline da Sprint 1: evidência e inspeção sintética, sem firmware real."""
import copy
import hashlib
import importlib.util
import tempfile
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SPEC = importlib.util.spec_from_file_location("platform_audit", ROOT / "tools" / "platform_audit.py")
audit = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(audit)


class PlatformAuditTests(unittest.TestCase):
    def setUp(self):
        self.doc = audit.load_inventory()

    def test_inventory_is_consistent_but_binary_unidentified(self):
        summary = audit.validate_inventory(self.doc)
        self.assertFalse(summary["base_sha_known"])
        self.assertGreaterEqual(summary["hooks"], 5)
        self.assertEqual(summary["regions"], 3)

    def test_reject_wrong_platform(self):
        doc = copy.deepcopy(self.doc)
        doc["platform"]["id"] = "5D2.212"
        with self.assertRaises(audit.AuditError):
            audit.validate_inventory(doc)

    def test_reject_unpinned_source(self):
        doc = copy.deepcopy(self.doc)
        doc["sources"]["soup"]["url"] = "https://github.com/jiijii0/soup-engine/tree/master"
        with self.assertRaises(audit.AuditError):
            audit.validate_inventory(doc)

    def test_reject_overlapping_regions(self):
        doc = copy.deepcopy(self.doc)
        doc["regions"][1]["start"] = "0x009D0000"
        with self.assertRaises(audit.AuditError):
            audit.validate_inventory(doc)

    def test_reject_duplicate_hook(self):
        doc = copy.deepcopy(self.doc)
        doc["hooks"].append(copy.deepcopy(doc["hooks"][0]))
        with self.assertRaises(audit.AuditError):
            audit.validate_inventory(doc)

    def test_reject_unverified_hash_misrepresented_as_identified(self):
        doc = copy.deepcopy(self.doc)
        doc["base_binary"]["verification"] = "identified"
        with self.assertRaises(audit.AuditError):
            audit.validate_inventory(doc)

    def test_reject_bad_rom_address(self):
        doc = copy.deepcopy(self.doc)
        doc["rom_functions"][0]["address"] = "0x00980000"
        with self.assertRaises(audit.AuditError):
            audit.validate_inventory(doc)

    def test_synthetic_autoexec_inspection(self):
        base = self.doc["base_binary"]
        b = bytearray(base["expected_size_bytes"])
        b[4:12] = b"gaonisoy"
        blob = bytes(b)
        expected = hashlib.sha256(blob).hexdigest()
        result = audit.inspect_autoexec_bytes(blob, expected, base)
        self.assertEqual(result["sha256"], expected)
        self.assertIn("sem homologação", result["result"])
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "synthetic-autoexec.bin"
            path.write_bytes(blob)
            self.assertEqual(audit.inspect_file(path, expected, base)["size"], len(blob))
            self.assertEqual(path.read_bytes(), blob)

    def test_reject_wrong_size_marker_and_hash(self):
        base = self.doc["base_binary"]
        blob = bytearray(base["expected_size_bytes"])
        blob[4:12] = b"gaonisoy"
        good = bytes(blob)
        expected = hashlib.sha256(good).hexdigest()
        for wrong in (good[:-1], good[:4] + b"badmark!" + good[12:]):
            with self.subTest(size=len(wrong)):
                with self.assertRaises(audit.AuditError):
                    audit.inspect_autoexec_bytes(wrong, expected, base)
        with self.assertRaises(audit.AuditError):
            audit.inspect_autoexec_bytes(good, "0" * 64, base)
        with self.assertRaises(audit.AuditError):
            audit.inspect_autoexec_bytes(good, "indefinido", base)


if __name__ == "__main__":
    unittest.main()
