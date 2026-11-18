import sys
import unittest
from pathlib import Path
sys.path.insert(0, str(Path(__file__).parents[1] / "tools"))
from importlib.machinery import SourceFileLoader
from importlib.util import spec_from_loader, module_from_spec
loader = SourceFileLoader("rb_report", str(Path(__file__).parents[1] / "tools/read-results.py"))
spec = spec_from_loader(loader.name, loader)
module = module_from_spec(spec)
loader.exec_module(module)

class ReportTests(unittest.TestCase):
    def test_event_parser(self):
        data = "EV_00000000_TYPE=00000000_PARAM=00000003\nEV_00000001_TYPE=00000000_PARAM=0000000B\n"
        self.assertEqual(module.parse_events(data),[(0,0,3,"MENU"),(1,0,11,"ZOOM_OUT_RELEASE?")])
    def test_non_button_and_bad_lines(self):
        data="EV_00000000_TYPE=00000001_PARAM=00000003\nBAD_LINE=NO\n"
        self.assertEqual(module.parse_events(data),[(0,1,3,"NON_BUTTON")])
