#!/usr/bin/env python3
"""Validador offline do payload Rebobina legado. NAO homologa uso na camera."""
import argparse
import hashlib
import json
import re
import struct
import subprocess
from pathlib import Path

ORIGIN = 0x009D0000
MAX_READ = 0x28000
MAX_MEMORY = 0x20000


def branch_target(code, offset):
    opcode, = struct.unpack_from('<I', code, offset)
    if (opcode & 0xFF000000) != 0xEA000000:
        raise ValueError(f'branch ARM esperado no offset 0x{offset:X}: {opcode:08X}')
    imm24 = opcode & 0xFFFFFF
    if imm24 & 0x800000:
        imm24 -= 1 << 24
    return offset + 8 + 4 * imm24


def validate(data, symbols):
    if not 0x100 <= len(data) <= MAX_MEMORY:
        raise ValueError('tamanho invalido para a regiao de memoria prevista')
    if data[:4] != b'FILM':
        raise ValueError('assinatura FILM ausente')
    for offset, name in ((4, 'rb_boot'), (0x44, 'rb_key')):
        actual = branch_target(data, offset)
        if name not in symbols:
            raise ValueError(f'simbolo nao encontrado: {name}')
        expected = symbols[name] - ORIGIN
        if actual != expected:
            raise ValueError(f'{name}: branch aponta 0x{actual:X}, esperado 0x{expected:X}')
        if not 0x100 <= actual < len(data):
            raise ValueError(f'{name} fora do payload')
    if len(data) > MAX_READ:
        raise ValueError('binario excede limite do loader legado')
    return {'size': len(data), 'sha256': hashlib.sha256(data).hexdigest(), 'mode': 'OFFLINE-ONLY'}


def load_symbols(elf):
    proc = subprocess.run(['readelf', '-Ws', str(elf)], capture_output=True,
                          text=True, check=True)
    syms = {}
    for line in proc.stdout.splitlines():
        bits = line.split()
        if len(bits) >= 8 and bits[0].endswith(':') and re.fullmatch(r'[0-9a-fA-F]+', bits[1]):
            syms[bits[-1]] = int(bits[1], 16)
    return syms


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--bin', required=True, type=Path)
    ap.add_argument('--elf', required=True, type=Path)
    ap.add_argument('--manifest', type=Path)
    args = ap.parse_args()
    data = args.bin.read_bytes()
    symbols = load_symbols(args.elf)
    info = validate(data, symbols)
    h = subprocess.run(['readelf', '-h', str(args.elf)], text=True,
                       capture_output=True, check=True).stdout
    if not re.search(r'Machine:\s+ARM\b', h) or not re.search(r'Type:\s+EXEC\b', h):
        raise ValueError('esperado ELF ARM executavel (intermediario, nao firmware)')
    r = subprocess.run(['readelf', '-r', str(args.elf)], text=True,
                       capture_output=True, check=True).stdout
    if 'There are no relocations in this file.' not in r:
        raise ValueError('relocacoes restantes no ELF final')
    if args.manifest:
        metadata = {
            'product': 'Rebobina — boot probe',
            'version': '0.0.1',
            'target': '5DC.111',
            'loader': 'patched Soup Engine 5D Classic AUTOEXEC.BIN',
            'sha256': info['sha256'],
            'size': info['size'],
            'rb_boot_offset': symbols['rb_boot'] - ORIGIN,
            'rb_key_offset': symbols['rb_key'] - ORIGIN,
            'hardware_verified': False,
            'warning': 'Somente prova estrutural: nao validado fisicamente em camera.'
        }
        args.manifest.write_text(json.dumps(metadata, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')
    print('Payload de teste estruturalmente valido:', info)
    print('ATENCAO: NAO foi testado em Canon EOS 5D real. NAO e firmware pronto.')


if __name__ == '__main__':
    main()
