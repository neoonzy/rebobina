import sys
import unittest
from pathlib import Path
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
from validate_probe import validate, branch_target, ORIGIN


class PayloadTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        root = Path(__file__).resolve().parents[1]
        cls.bin = (root / 'dist/SE.BIN').read_bytes()
        from validate_probe import load_symbols
        cls.symbols = load_symbols(root / 'build/rebobina-probe.elf')

    def test_valid_payload(self):
        self.assertEqual(validate(self.bin, self.symbols)['mode'], 'OFFLINE-ONLY')

    def test_header(self):
        self.assertEqual(self.bin[:4], b'FILM')
        self.assertGreaterEqual(len(self.bin), 0x100)

    def test_branch_targets(self):
        self.assertEqual(branch_target(self.bin, 4), self.symbols['rb_boot'] - ORIGIN)
        self.assertEqual(branch_target(self.bin, 0x44), self.symbols['rb_key'] - ORIGIN)

    def test_reject_bad_signature(self):
        with self.assertRaises(ValueError):
            validate(b'FAIL' + self.bin[4:], self.symbols)

    def test_reject_short_file(self):
        with self.assertRaises(ValueError):
            validate(self.bin[:20], self.symbols)

    def test_reject_corrupt_entry(self):
        damaged = bytearray(self.bin)
        damaged[4:8] = b'\x00' * 4
        with self.assertRaises(ValueError):
            validate(bytes(damaged), self.symbols)


if __name__ == '__main__':
    unittest.main()
