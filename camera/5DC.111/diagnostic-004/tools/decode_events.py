#!/usr/bin/env python3
"""Decode REBOBINA 0.0.4 passive GUI events on a PC. Read-only."""
import argparse
import re
from pathlib import Path

# GPLv2 historical research: Magic Lantern Team, platform/5DC.111/gui.h
# https://github.com/frantony/magic-lantern/blob/a3454ebed3f6adcd165ee9382f451f917d989253/platform/5DC.111/gui.h
EVENTS = {
    0x00: "WHEEL_UP", 0x01: "WHEEL_DOWN",
    0x02: "SET", 0x03: "MENU", 0x04: "INFO", 0x05: "JUMP",
    0x06: "PLAY", 0x07: "TRASH",
    0x08: "ZOOM_IN_PRESS_MAYBE",
    0x09: "ZOOM_IN_RELEASE_MAYBE",
    0x0A: "ZOOM_OUT_PRESS_MAYBE",
    0x0B: "ZOOM_OUT_RELEASE_MAYBE",
    0x0C: "DIRECT_PRINT_PRESS",
    0x0D: "DIRECT_PRINT_RELEASE",
    0x0E: "UDLR_RELEASE",
    0x0F: "UP", 0x10: "UP_RIGHT", 0x11: "UP_LEFT",
    0x12: "RIGHT", 0x13: "LEFT",
    0x14: "DOWN_RIGHT", 0x15: "DOWN_LEFT", 0x16: "DOWN",
}
PATTERN = re.compile(r"^(RAW_EVENT|EVENT_PARAM)_([0-9A-Fa-f]{8})=([0-9A-Fa-f]{8})$")

def decode(text):
    """Return (event count if present, rows of (index, type, param, label))."""
    groups = {}
    total = None
    for line in text.splitlines():
        line = line.strip()
        if line.startswith("GUI_EVENTS_HEX="):
            total = int(line.split("=", 1)[1], 16)
        m = PATTERN.fullmatch(line)
        if m is None:
            continue
        field, index, value = m.groups()
        groups.setdefault(int(index, 16), {})[field] = int(value, 16)
    rows = []
    for index, entry in sorted(groups.items()):
        kind, param = entry.get("RAW_EVENT"), entry.get("EVENT_PARAM")
        label = (EVENTS.get(param, "UNKNOWN") if kind == 0 and param is not None
                 else "UNDECODED")
        rows.append((index, kind, param, label))
    return total, rows

def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("path", type=Path, help="RB4LATE.TXT (read-only)")
    args = ap.parse_args()
    total, rows = decode(args.path.read_text(encoding="ascii"))
    print("Reported events:", total if total is not None else "unknown")
    print("Captured rows:", len(rows), "(up to the first 12 events)")
    print("IDX  TYPE       PARAM      HISTORICAL_MAPPING")
    for index, kind, param, label in rows:
        k = "--------" if kind is None else f"{kind:08X}"
        p = "--------" if param is None else f"{param:08X}"
        print(f"{index:03d}  {k}   {p}   {label}")
    if total is not None and total > len(rows):
        print("Note: the camera stores only the first 12 events.")
    print("MAYBE labels are tentative; physical button order was not supplied.")

if __name__ == "__main__":
    main()
