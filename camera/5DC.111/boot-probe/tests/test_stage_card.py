import struct
import sys
import tempfile
import unittest
from pathlib import Path
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
from stage_card import inspect_legacy_loader, fileoff, GUI_SITE, INIT_LITERAL, STUB_LO, make_staging


def dummy_soup_loader():
    data = bytearray(221184)
    data[4:12] = b'gaonisoy'
    struct.pack_into('<I', data, fileoff(INIT_LITERAL), STUB_LO + 0x80)
    imm = ((STUB_LO + 0x90) - GUI_SITE - 8) // 4
    struct.pack_into('<I', data, fileoff(GUI_SITE), 0xEB000000 | (imm & 0xFFFFFF))
    data[fileoff(STUB_LO):fileoff(STUB_LO)+9] = b'A:/SE.BIN'
    return bytes(data)


class CardStageTests(unittest.TestCase):
    def test_detect_synthetic_patch(self):
        self.assertEqual(inspect_legacy_loader(dummy_soup_loader())['init'], hex(STUB_LO + 0x80))

    def test_reject_unpatched_or_bad_signature(self):
        for original in (bytes(221184), b'gaonisoy'):
            with self.subTest(length=len(original)), self.assertRaises(ValueError):
                inspect_legacy_loader(original)

    def test_reject_bad_gui_branch(self):
        image = bytearray(dummy_soup_loader())
        struct.pack_into('<I', image, fileoff(GUI_SITE), 0)
        with self.assertRaises(ValueError):
            inspect_legacy_loader(bytes(image))

    def test_staging_keeps_original_untouched(self):
        root = Path(__file__).resolve().parents[1]
        with tempfile.TemporaryDirectory() as tmp:
            d = Path(tmp)
            source = d / 'soup' / 'card'
            (source / 'ML').mkdir(parents=True)
            (source / 'AUTOEXEC.BIN').write_bytes(dummy_soup_loader())
            (source / 'SE.BIN').write_bytes(b'FILM' + b'X' * 300)
            orig = (source / 'SE.BIN').read_bytes()
            out = d / 'output' / 'card-staging'
            result = make_staging(source, root / 'dist/SE.BIN', out)
            self.assertEqual((source / 'SE.BIN').read_bytes(), orig)
            self.assertEqual((out / 'SE.BIN').read_bytes(), (root / 'dist/SE.BIN').read_bytes())
            self.assertNotEqual(result['base']['source_sha256'], result['rebobina']['sha256'])
            with self.assertRaises(ValueError):
                make_staging(source, root / 'dist/SE.BIN', out)


if __name__ == '__main__':
    unittest.main()
