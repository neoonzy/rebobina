import sys
import unittest
from pathlib import Path
sys.path.insert(0, str(Path(__file__).parents[1] / 'tools'))
from validate_probe import branch, validate, BASE

class TestHeader(unittest.TestCase):
    @staticmethod
    def good():
        d=bytearray(512)
        d[:4]=b'FILM'
        d[4:8]=((0xea000000 | ((0x100-4-8)//4))).to_bytes(4,'little')
        d[0x44:0x48]=((0xea000000 | ((0x120-0x44-8)//4))).to_bytes(4,'little')
        d[0x140:0x140+len(b'REBOBINA 0.1.0 DEV / BOOT')]=b'REBOBINA 0.1.0 DEV / BOOT'
        return d
    def test_good(self):
        self.assertEqual(validate(self.good(),{'rb_boot':BASE+0x100,'rb_key':BASE+0x120})['size'],512)
    def test_bad_magic(self):
        d=self.good(); d[:4]=b'NOPE'
        with self.assertRaises(ValueError): validate(d,{'rb_boot':BASE+0x100,'rb_key':BASE+0x120})
    def test_wrong_entry(self):
        with self.assertRaises(ValueError): validate(self.good(),{'rb_boot':BASE+0x110,'rb_key':BASE+0x120})
    def test_bad_key(self):
        d=self.good(); d[0x44:0x48]=b'ABCD'
        with self.assertRaises(ValueError): validate(d,{'rb_boot':BASE+0x100,'rb_key':BASE+0x120})
    def test_missing_version(self):
        d=self.good(); d[0x140:0x165]=b'\0'*(0x165-0x140)
        with self.assertRaises(ValueError): validate(d,{'rb_boot':BASE+0x100,'rb_key':BASE+0x120})
    def test_short(self):
        with self.assertRaises(ValueError): validate(b'FILM',{})
if __name__=='__main__': unittest.main()
