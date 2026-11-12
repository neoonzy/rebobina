#!/usr/bin/env python3
"""Read-only audit of Magic Lantern 5DC.111 source reuse; no camera I/O."""
import argparse
import json
import re
import subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "camera/5DC.111/ml_compat"
PIN = "a3454ebed3f6adcd165ee9382f451f917d989253"


def audit(elf=None):
    data = json.loads((SOURCE / "source.json").read_text(encoding="utf-8"))
    assert data["target"] == "5DC.111"
    assert data["upstream"]["commit"] == PIN
    assert data["hardware_verified"] is False
    assert data["base_2012_ml_sha256"] is None
    assert data["scope"] == "offline-only"

    asm = (SOURCE / "rom_stubs.S").read_text(encoding="utf-8")
    entries = re.findall(
        r"^NSTUB\(\s*(0x[0-9a-fA-F]+)\s*,\s*(\w+)\s*\)", asm, re.M
    )
    found = {name: int(addr, 16) for addr, name in entries}
    expected = {key: int(value, 16) for key, value in data["rom_symbols"].items()}
    assert len(found) == len(entries) == 10
    assert found == expected, "source ROM stubs differ from pinned metadata"
    assert "Magic Lantern Team" in asm and "NO WARRANTY" in asm

    header = (SOURCE / "input.h").read_text(encoding="utf-8")
    assert "Magic Lantern Team" in header
    for key, value in data["button_codes"].items():
        match = re.search(r"^#define\s+" + re.escape(key) +
                          r"\s+(0x[0-9a-fA-F]+)u\b", header, re.M)
        assert match is not None and int(match.group(1), 16) == int(value, 16)

    if elf:
        proc = subprocess.run(["readelf", "-Ws", str(elf)], check=True,
                              capture_output=True, text=True)
        symbols = {}
        for line in proc.stdout.splitlines():
            parts = line.split()
            if (len(parts) >= 8 and parts[3] == "NOTYPE" and
                    parts[4] == "GLOBAL" and parts[6] == "ABS"):
                symbols[parts[7]] = int(parts[1], 16)
        assert symbols == expected, f"unexpected ELF ABS symbols: {symbols}"

    return {"mode": "OFFLINE_ONLY", "rom_symbols": len(found),
            "button_events": len(data["button_codes"]), "upstream_commit": PIN}


if __name__ == "__main__":
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("--elf", type=Path)
    args = p.parse_args()
    print(json.dumps(audit(args.elf), indent=2))
