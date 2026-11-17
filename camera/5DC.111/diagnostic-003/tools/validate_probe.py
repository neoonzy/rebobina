#!/usr/bin/env python3
"""Offline check for the 5DC Soup loader ABI; cannot validate hardware safety."""
import argparse
import hashlib
import json
import re
import struct
import subprocess
from pathlib import Path

BASE = 0x009D0000
MAX_SIZE = 0x20000

def branch(data, offset):
    insn, = struct.unpack_from('<I', data, offset)
    if insn & 0xff000000 != 0xea000000:
        raise ValueError(f'expected unconditional ARM B at 0x{offset:x}')
    delta = insn & 0xffffff
    if delta & 0x800000:
        delta -= 0x1000000
    return offset + 8 + 4*delta

def validate(data, symbols):
    if not 0x100 <= len(data) <= MAX_SIZE:
        raise ValueError('payload outside allowed range')
    if data[:4] != b'FILM':
        raise ValueError('FILM signature missing')
    for off, sym in ((4, 'rb_boot'), (0x44, 'rb_key')):
        target = branch(data, off)
        if sym not in symbols or target != symbols[sym] - BASE:
            raise ValueError(f'{sym} branch mismatch')
        if not 0x100 <= target < len(data):
            raise ValueError(f'{sym} target outside payload')
    if b'REBOBINA BOOT DIAGNOSTICO 0.0.3' not in data:
        raise ValueError('version string missing')
    return {'sha256': hashlib.sha256(data).hexdigest(), 'size':len(data),
            'entry_offset':branch(data,4), 'key_offset':branch(data,0x44)}

def inspect(elf):
    header = subprocess.check_output(['readelf', '-h', str(elf)], text=True)
    if not re.search(r'Machine:\s+ARM\b',header) or not re.search(r'Type:\s+EXEC\b',header):
        raise ValueError('wrong ELF target')
    secs = subprocess.check_output(['readelf','-W','-S',str(elf)],text=True)
    for line in secs.splitlines():
        if re.search(r'\s\.bss\s',line):
            cells = line.split()
            # readelf -W -S: [Nr] Name Type Address Off Size ...
            m = re.search(r'\s\.bss\s+NOBITS\s+[0-9A-Fa-f]+\s+[0-9A-Fa-f]+\s+([0-9A-Fa-f]+)\s',line)
            if not m or int(m.group(1),16):
                raise ValueError('nonzero .bss would not be initialized')
    rel = subprocess.check_output(['readelf','-r',str(elf)],text=True)
    if 'There are no relocations in this file.' not in rel:
        raise ValueError('unexpected dynamic relocations')
    syms = {}
    out = subprocess.check_output(['readelf','-Ws',str(elf)],text=True)
    for line in out.splitlines():
        col=line.split()
        if len(col)>=8 and col[0].endswith(':') and re.fullmatch('[0-9A-Fa-f]+',col[1]):
            syms[col[-1]]=int(col[1],16)
    return syms

def main():
    p=argparse.ArgumentParser()
    p.add_argument('--bin',type=Path,required=True)
    p.add_argument('--elf',type=Path,required=True)
    p.add_argument('--manifest',type=Path)
    a=p.parse_args()
    result=validate(a.bin.read_bytes(),inspect(a.elf))
    result.update({'product':'Rebobina 5DC diagnostic','version':'0.0.3',
                   'target':'5DC.111','loader':'Soup Engine 5DC patched AUTOEXEC.BIN',
                   'tested_on_hardware':False,'modifies_autoexec':False})
    if a.manifest:
        a.manifest.write_text(json.dumps(result,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    print('OFFLINE VALIDATED:',result)
    print('CAUTION: on-camera behavior NOT validated by this check.')
if __name__=='__main__': main()
