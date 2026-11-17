"""REBOBINA 0.0.4 on-host report decoder tests; no hardware access."""
import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "tools"))
from decode_events import decode

class TestDecode(unittest.TestCase):
    def test_real_camera_report(self):
        data = """REBOBINA DIAGNOSTICO 0.0.4 / PASSIVE KEY TEST
READBACK=PASS
GUI_EVENTS_HEX=00000009
RAW_EVENT_00000000=00000000
EVENT_PARAM_00000000=00000003
RAW_EVENT_00000001=00000000
EVENT_PARAM_00000001=0000000B
RAW_EVENT_00000002=00000000
EVENT_PARAM_00000002=00000009
RAW_EVENT_00000003=00000000
EVENT_PARAM_00000003=00000004
RAW_EVENT_00000004=00000000
EVENT_PARAM_00000004=0000000B
RAW_EVENT_00000005=00000000
EVENT_PARAM_00000005=00000009
RAW_EVENT_00000006=00000000
EVENT_PARAM_00000006=00000002
RAW_EVENT_00000007=00000000
EVENT_PARAM_00000007=00000002
RAW_EVENT_00000008=00000000
EVENT_PARAM_00000008=00000007
LCD=OPT_IN_DISABLED
"""
        count, rows = decode(data)
        self.assertEqual(count, 9)
        self.assertEqual(len(rows), 9)
        self.assertEqual([row[2] for row in rows],
                         [3, 11, 9, 4, 11, 9, 2, 2, 7])
        self.assertEqual(rows[0][3], "MENU")
        self.assertEqual(rows[-1][3], "TRASH")
        self.assertIn("MAYBE", rows[1][3])

    def test_non_gui_and_incomplete_event(self):
        count, rows = decode("GUI_EVENTS_HEX=00000001\nRAW_EVENT_00000000=00000001\nEVENT_PARAM_00000000=00000003\n")
        self.assertEqual(count, 1)
        self.assertEqual(rows[0][3], "UNDECODED")
        count, rows = decode("EVENT_PARAM_00000002=00000004\n")
        self.assertIsNone(count)
        self.assertEqual(rows[0][3], "UNDECODED")

    def test_unknown_code_and_blank(self):
        count, rows = decode("RAW_EVENT_00000000=00000000\nEVENT_PARAM_00000000=000000FE\n")
        self.assertEqual(rows[0][3], "UNKNOWN")
        self.assertEqual(decode(""), (None, []))

if __name__ == "__main__":
    unittest.main()
