#!/usr/bin/env python3
"""Offline boot-chain evidence audit; no firmware, CF or camera I/O."""
import json
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
DATA = ROOT / "docs/data/5dc-boot-chain-research.json"
HEADER = ROOT / "camera/5DC.111/bootstrap/boot_chain.h"
PIN = "a3454ebed3f6adcd165ee9382f451f917d989253"


def validate_data(doc):
    if doc.get("platform") != "5DC.111" or doc.get("firmware") != "1.1.1":
        raise ValueError("wrong 5D Classic firmware")
    source = doc.get("source", {})
    if source.get("repo") != "frantony/magic-lantern" or source.get("commit") != PIN:
        raise ValueError("unidentified historical Magic Lantern source")
    if source.get("installer_include_in_top_level_build") is not False:
        raise ValueError("unsupported installer incorrectly treated as official build")
    if source.get("license") != "GPLv2":
        raise ValueError("missing Magic Lantern license")
    roles = doc.get("historical_address_roles", [])
    if len(roles) != 2 or {x.get("role") for x in roles} != {
        "installer_raw_load", "ml_relocated_runtime"
    }:
        raise ValueError("wrong boot address roles")
    addresses = {r["role"]: int(r["value"], 16) for r in roles}
    if addresses["installer_raw_load"] != 0x800000 or \
            addresses["ml_relocated_runtime"] != 0x980000 or \
            addresses["installer_raw_load"] == addresses["ml_relocated_runtime"]:
        raise ValueError("historical load and relocation wrongly conflated")
    if any(r.get("hardware_verified") is not False for r in roles):
        raise ValueError("unverified historical addresses must not be marked hardware proven")

    flags = doc.get("bootflags", {})
    wanted = {
        "flash_mapping": 0xF8000000,
        "read_entry": 0xFFFF8AE0,
        "read_opcode": 0xE52DE004,
        "write_entry": 0xFFFF89F0,
        "write_opcode": 0xE92D41F0,
        "card_bootflag_writer": 0xFFFF4074,
    }
    try:
        found = {key: int(flags[key], 16) for key in wanted}
    except (KeyError, TypeError, ValueError) as exc:
        raise ValueError("invalid bootflag research metadata") from exc
    if found != wanted:
        raise ValueError("bootloader research differs from pinned installer source")

    if doc.get("camera_installation_authorized") is not False or \
            doc.get("camera_boot_verified") is not False or \
            doc.get("physical_boot_recovery_verified") is not False:
        raise ValueError("offline evidence may not claim hardware authorization")
    if doc.get("offline_preflight_status") != "RESEARCH_ONLY":
        raise ValueError("offline test status must remain research-only")
    if doc.get("output_allowlist") != ["build/arm/5dc-stage0-offline.elf"]:
        raise ValueError("unexpected installable firmware output")

    return addresses, wanted


def audit():
    doc = json.loads(DATA.read_text(encoding="utf-8"))
    addresses, addresses_rom = validate_data(doc)
    h = HEADER.read_text(encoding="utf-8")
    expected = {
        "RB_5DC_HISTORIC_LOADER_START": addresses["installer_raw_load"],
        "RB_5DC_HISTORIC_ML_RELOCATED": addresses["ml_relocated_runtime"],
        "RB_5DC_HISTORIC_ROM_READ_PROLOGUE": addresses_rom["read_opcode"],
        "RB_5DC_HISTORIC_ROM_WRITE_PROLOGUE": addresses_rom["write_opcode"],
    }
    for name, value in expected.items():
        match = re.search(
            r"^#define\s+" + name + r"\s+UINT32_C\((0x[0-9a-fA-F]+)\)",
            h, re.M,
        )
        if match is None or int(match.group(1), 16) != value:
            raise ValueError(f"{name} does not match pinned source metadata")

    # Bootstrap must not contain a real installer or ROM writer.
    for forbidden in ("write_bootflag(", "write_card_bootflag(",
                      "EnableBootDisk(", "FIO_WriteFile(", "FIO_CreateFile("):
        for path in (ROOT / "camera/5DC.111/bootstrap").glob("*.c"):
            if forbidden in path.read_text(encoding="utf-8"):
                raise ValueError(f"unexpected camera write path in {path.name}")
    print("OK: 5DC.111 Magic Lantern boot research pinned; 0x800000 != 0x980000; no installer")
    return True


if __name__ == "__main__":
    audit()
