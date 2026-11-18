"""Read-only host reporting tool smoke tests."""
import contextlib
import io
import sys
import tempfile
import unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
import importlib.util
spec=importlib.util.spec_from_file_location('read_results',Path(__file__).resolve().parents[1]/'tools'/'read-results.py')
mod=importlib.util.module_from_spec(spec)
spec.loader.exec_module(mod)

class ReportTests(unittest.TestCase):
 def test_report_covered(self):
  with tempfile.TemporaryDirectory() as d:
   base=Path(d)
   (base/'RB8BOOT.TXT').write_text('REBOBINA 0.2.0 BETA / BOOT\n')
   (base/'RB8TEST.TXT').write_text('WRITE+READBACK+VERIFY\n')
   old=sys.argv
   try:
    sys.argv=['read-results.py',str(base)]
    out=io.StringIO()
    with contextlib.redirect_stdout(out):mod.main()
   finally:sys.argv=old
   text=out.getvalue()
   self.assertIn('RB8BOOT.TXT',text)
   self.assertIn('WRITE+READBACK+VERIFY',text)
   self.assertIn('Nenhuma configuracao',text)
   self.assertIn('SESSION exige',text)

if __name__=='__main__':unittest.main()
