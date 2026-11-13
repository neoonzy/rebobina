#!/usr/bin/env python3
"""Fail-closed audit of the independent 5DC ARM stage0 OFFLINE ELF.

No camera, CF, USB, ROM, firmware file or boot flag is ever accessed.
The resulting ELF is NOT installable and must NEVER be converted to BIN.
"""
import argparse
import re
import subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
BOOT = ROOT / "camera/5DC.111/bootstrap"
EXPECTED_START = 0x00980000
HISTORICAL_LIMIT = 0x00A00000
EXPECTED_SYMBOLS = {
    "rb_stage0_entry", "rb_stage0_prepare", "rb_stage0_check",
    "rb_registry_init", "rb_ui_init", "rb_stage0_warning",
    "__rb_stage0_image_start", "__rb_stage0_image_end",
    "__rb_stage0_stack_top", "__rb_stage0_bss_start", "__rb_stage0_bss_end",
}


def _readelf(*args):
    return subprocess.run(["readelf", "-W", *map(str, args)],
                          capture_output=True, text=True, check=True).stdout


def audit(path):
    path = Path(path)
    if path.suffix.lower() != ".elf" or not path.name.endswith("-offline.elf"):
        raise ValueError("Expected a specially named OFFLINE ELF, never .BIN or .FIR")
    raw = path.read_bytes()
    if raw[:4] != b"\x7fELF" or raw[4] != 1 or raw[5] != 1:
        raise ValueError("Expected little-endian ELF32")
    if b"REBOBINA 5DC STAGE0 OFFLINE ONLY" not in raw:
        raise ValueError("Mandatory offline-only marker missing")

    hdr = _readelf("-h", path)
    if not re.search(r"Type:\s+EXEC\b", hdr):
        raise ValueError("Expected explicitly linked ARM ELF executable")
    if not re.search(r"Machine:\s+ARM\b", hdr):
        raise ValueError("Wrong machine")
    match = re.search(r"Entry point address:\s*(0x[0-9a-fA-F]+)", hdr)
    if not match or int(match.group(1), 16) != EXPECTED_START:
        raise ValueError("Offline entry differs from the historical address")

    symbols = _readelf("-s", path)
    names = set()
    undefined = []
    values = {}
    for line in symbols.splitlines():
        parts = line.split()
        if len(parts) < 8 or not parts[0].rstrip(":").isdigit():
            continue
        value, binding, ndx, name = parts[1], parts[4], parts[6], parts[7]
        if binding == "GLOBAL" and ndx == "UND":
            undefined.append(name)
        names.add(name)
        if name in EXPECTED_SYMBOLS:
            values[name] = int(value, 16)

    missing = EXPECTED_SYMBOLS - names
    if missing:
        raise ValueError(f"Missing bootstrap/core symbols: {sorted(missing)}")
    if undefined:
        raise ValueError(f"Undefined link-time dependencies: {undefined}")
    if values.get("rb_stage0_entry") != EXPECTED_START:
        raise ValueError("Entry symbol does not match ELF entry")
    if not EXPECTED_START <= values["__rb_stage0_image_start"] < values["__rb_stage0_image_end"] <= HISTORICAL_LIMIT:
        raise ValueError("Prototype sections/stack exceed historical memory hypothesis")
    if not values["__rb_stage0_bss_start"] <= values["__rb_stage0_bss_end"] <= values["__rb_stage0_stack_top"]:
        raise ValueError("Prototype BSS/stack order invalid")

    program = _readelf("-l", path)
    segments = []
    for line in program.splitlines():
        parts = line.split()
        if not parts or parts[0] != "LOAD":
            continue
        if len(parts) < 6:
            raise ValueError("Unrecognized ELF program header")
        # readelf -W -l: Type Offset VirtAddr PhysAddr FileSiz MemSiz ...
        addr = int(parts[2], 16)
        size = int(parts[5], 16)
        if not EXPECTED_START <= addr < HISTORICAL_LIMIT or addr + size > HISTORICAL_LIMIT:
            raise ValueError("A loadable segment escapes the historical hypothesis")
        segments.append((addr, size))
    if not segments:
        raise ValueError("No loadable ELF segments")

    # Stage0 must contain no Canon ROM calls or Magic Lantern loader sources.
    boot_c = (BOOT / "stage0.c").read_text(encoding="utf-8")
    boot_asm = (BOOT / "stage0_entry.S").read_text(encoding="utf-8")
    for forbidden in ("cache_fake(", "FIO_OpenFile(", "firmware_entry(", "EnableBootDisk("):
        if forbidden in boot_c or forbidden in boot_asm:
            raise ValueError(f"Unexpected camera-side operation: {forbidden}")
    return {"status": "PASS_OFFLINE_ONLY", "elf": str(path),
            "entry": hex(EXPECTED_START), "segments": len(segments),
            "memory_end": hex(values["__rb_stage0_image_end"]),
            "magic_lantern_preinstall_required": False,
            "camera_boot_validated": False,
            "camera_installable": False}


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("elf", type=Path)
    args = parser.parse_args()
    print(audit(args.elf))
