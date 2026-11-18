"""Binary format and host processing adapter tests; no camera/CF writes."""
import os
import sys
import struct
import tempfile
import unittest
from pathlib import Path
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
from config_reader import parse_config, read_latest, fnv32
from develop_selected import prepare


def record(sequence, film=1, roll=3, frames=10):
    words=(0x52424346,1,sequence,film,roll,frames,3,5)
    return struct.pack('<9I', *words,fnv32(words))


class TestConfigReader(unittest.TestCase):
    def test_valid_and_invalid(self):
        self.assertEqual(parse_config(record(1))['profile'],'PORTRA')
        self.assertEqual(parse_config(record(1))['roll'],3)
        self.assertIsNone(parse_config(record(1)[:35]))
        s=bytearray(record(1));s[12]^=0x20
        self.assertIsNone(parse_config(bytes(s)))
        self.assertIsNone(parse_config(record(1,film=99)))
    def test_slot_sequence_and_fallback(self):
        with tempfile.TemporaryDirectory() as d:
            root=Path(d);(root/'ML').mkdir()
            (root/'ML'/'RB8CFG0.BIN').write_bytes(record(1,film=0))
            (root/'ML'/'RB8CFG1.BIN').write_bytes(record(2,film=2))
            self.assertEqual(read_latest(root)['profile'],'ACROS')
            (root/'ML'/'RB8CFG1.BIN').write_bytes(b'bad')
            self.assertEqual(read_latest(root)['profile'],'SOUP 800')
    def test_host_adapter_doesnt_overwrite(self):
        with tempfile.TemporaryDirectory() as d:
            root=Path(d);card=root/'card';(card/'ML').mkdir(parents=True)
            (card/'ML'/'RB8CFG1.BIN').write_bytes(record(1,film=1))
            src=root/'photo.CR2';src.write_bytes(b'CR2 fake content')
            out=root/'out';out.mkdir();stocks=root/'stocks';stocks.mkdir()
            (stocks/'portra.fs').write_text('fake-stock')
            tool=root/'rebobina-cr2-develop';tool.write_text('fake binary')
            cmd,cfg=prepare(card,src,out,tool,stocks)
            self.assertEqual(cmd[4],'photo-RB-1.JPG')
            self.assertEqual(cfg['stock_file'],'portra.fs')
            (out/cmd[4]).write_text('already exists')
            with self.assertRaises(ValueError): prepare(card,src,out,tool,stocks)


if __name__ == '__main__':unittest.main()
