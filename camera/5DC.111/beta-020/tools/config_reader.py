#!/usr/bin/env python3
"""Read the 5D Classic Rebobina v1 configuration slots on a HOST (read-only).

The on-camera beta saves the chosen profile/roll in two alternating records.
No camera executable and no CF files are written by this module.
"""
from __future__ import annotations
import argparse
from pathlib import Path
import struct

CFG_MAGIC = 0x52424346
FMT = '<9I'
RECORD_SIZE = struct.calcsize(FMT)
PROFILE_NAMES = ('SOUP 800', 'PORTRA', 'ACROS', 'CINE (ETERNA)')
STOCK_FILES = ('soup800.fs', 'portra.fs', 'acros.fs', 'eterna.fs')


def fnv32(fields: tuple[int, ...]) -> int:
    h = 2166136261
    for value in fields:
        for shift in (0, 8, 16, 24):
            h = (h ^ ((value >> shift) & 0xff)) * 16777619 & 0xffffffff
    return h


def parse_config(data: bytes) -> dict | None:
    if len(data) != RECORD_SIZE:
        return None
    magic, version, seq, film, roll, frames, modules, flags, check = struct.unpack(FMT, data)
    if (magic != CFG_MAGIC or version != 1 or seq == 0 or film >= 4 or
            roll == 0 or roll > 9999 or frames > 36 or modules & ~3 or
            flags & ~7 or fnv32((magic,version,seq,film,roll,frames,modules,flags)) != check):
        return None
    return {'sequence': seq, 'film': film, 'profile': PROFILE_NAMES[film],
            'stock_file': STOCK_FILES[film], 'roll': roll, 'frames': frames,
            'modules': modules, 'flags': flags}


def read_latest(card_root: Path) -> dict | None:
    """Card root containing an ML directory; returns newest *valid* slot."""
    found = []
    for slot in (0, 1):
        path = card_root / 'ML' / f'RB8CFG{slot}.BIN'
        if path.is_file():
            try:
                parsed = parse_config(path.read_bytes())
            except OSError:
                parsed = None
            if parsed is not None:
                parsed['slot'] = slot
                found.append(parsed)
    if not found:
        return None
    if len(found) == 1:
        return found[0]
    a, b = found
    # Match ARM signed wrap-aware seq delta; most recently written record wins.
    delta = (b['sequence'] - a['sequence']) & 0xffffffff
    return b if delta != 0 and delta < 0x80000000 else a


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('card_root', type=Path, help='Root of the unmounted/counted CF card (e.g. D:\\ or /media/CF)')
    args = ap.parse_args()
    cfg = read_latest(args.card_root)
    if cfg is None:
        print('Nenhuma configuracao RB8 valida encontrada.')
        return 1
    for key, value in cfg.items():
        print(f'{key}: {value}')
    print('Observacao: os perfis ainda NAO sao processados pela camera.')
    return 0

if __name__ == '__main__':
    raise SystemExit(main())
