#!/usr/bin/env python3
"""Parse the integrated RB7 diagnostics. Read-only; no camera/CF writes."""
from argparse import ArgumentParser
from pathlib import Path
import re

BUTTONS = {
    0: "WHEEL_UP", 1: "WHEEL_DOWN", 2: "SET", 3: "MENU", 4: "INFO",
    5: "JUMP", 6: "PLAY", 7: "TRASH", 8: "ZOOM_IN?", 9: "ZOOM_IN_RELEASE?",
    10: "ZOOM_OUT?", 11: "ZOOM_OUT_RELEASE?", 12: "PRINT", 13: "PRINT_RELEASE",
    14: "UDLR_RELEASE", 15: "UP", 16: "UP_RIGHT", 17: "UP_LEFT", 18: "RIGHT",
    19: "LEFT", 20: "DOWN_RIGHT", 21: "DOWN_LEFT", 22: "DOWN"
}
EVENT = re.compile(r"^EV_([A-Fa-f0-9]{8})_TYPE=([A-Fa-f0-9]{8})_PARAM=([A-Fa-f0-9]{8})$")


def parse_events(text):
    matches = []
    for row in text.splitlines():
        m = EVENT.fullmatch(row.strip())
        if m:
            index, type_, param = (int(x, 16) for x in m.groups())
            label = BUTTONS.get(param, "UNKNOWN") if type_ == 0 else "NON_BUTTON"
            matches.append((index, type_, param, label))
    return matches


def main():
    p = ArgumentParser(description=__doc__)
    p.add_argument("folder", type=Path, help="Folder with RB7*.TXT logs")
    args = p.parse_args()
    for name in ("RB7BOOT.TXT", "RB7TEST.TXT", "RB7EVENT.TXT", "RB7FINAL.TXT"):
        path = args.folder / name
        print(f"\n--- {name} ---")
        if not path.exists():
            print("NOT FOUND")
            continue
        text = path.read_text(encoding="ascii", errors="replace")
        print(text)
        if name == "RB7EVENT.TXT":
            print("Decoded GUI events (historical 5DC map, zoom codes tentative):")
            for i, ty, code, label in parse_events(text):
                print(f"  #{i:02d} type={ty:08X} param={code:08X} {label}")
    print("\nNo files were modified.")


if __name__ == "__main__":
    main()
