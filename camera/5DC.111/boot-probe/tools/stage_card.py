#!/usr/bin/env python3
"""Monta uma pasta TEMPORARIA de teste. NUNCA grava diretamente no CompactFlash."""
import argparse
import hashlib
import json
import shutil
import struct
from pathlib import Path
from validate_probe import branch_target

ROOT = Path(__file__).resolve().parents[1]
STAGING = ROOT / 'dist' / 'card-staging'
INIT_LITERAL = 0x00980774
GUI_SITE = 0x00989DB4
STUB_LO = 0x00980820
STUB_HI = 0x0098090C
RELOC = 0x00980000
OFF = 0x1000


def fileoff(addr):
    return (addr - RELOC) + OFF


def target_arm_bl(insn, addr):
    if (insn & 0xFF000000) != 0xEB000000:
        raise ValueError('hook nao usa branch ARM BL incondicional conhecido')
    v = insn & 0xFFFFFF
    if v & 0x800000:
        v -= 0x1000000
    return addr + 8 + 4 * v


def inspect_legacy_loader(data):
    """Confere layout estrutural do patch Soup, nao hash independente da origem."""
    if len(data) != 221184:
        raise ValueError('AUTOEXEC.BIN nao tem o tamanho esperado para o patch 5DC 2012')
    if data[4:12] != b'gaonisoy':
        raise ValueError('AUTOEXEC.BIN nao tem marcador gaonisoy')
    init, = struct.unpack_from('<I', data, fileoff(INIT_LITERAL))
    if not STUB_LO <= init < STUB_HI:
        raise ValueError('nao e a variante Soup com boot delegado ao stub (init)')
    gui, = struct.unpack_from('<I', data, fileoff(GUI_SITE))
    target = target_arm_bl(gui, GUI_SITE)
    if not STUB_LO <= target < STUB_HI:
        raise ValueError('hook GUI nao aponta para o stub esperado')
    if b'A:/SE.BIN' not in data[fileoff(STUB_LO):fileoff(STUB_HI)]:
        raise ValueError('stub nao contem caminho de loader A:/SE.BIN')
    return {'source_sha256': hashlib.sha256(data).hexdigest(),
            'init': hex(init), 'gui_target': hex(target)}


def make_staging(soup_card, probe, output):
    soup_card = Path(soup_card)
    probe = Path(probe)
    output = Path(output)
    if not soup_card.is_dir():
        raise ValueError('informe a pasta card/ extraida do pacote oficial Soup Engine')
    if (soup_card / 'DCIM').exists():
        raise ValueError('origem parece ser cartao com fotografias; use uma pasta extraida do ZIP')
    if output.exists():
        raise ValueError('pasta staging ja existe; remova manualmente antes de repetir')
    if soup_card.resolve() == output.resolve() or output.resolve() in soup_card.resolve().parents:
        raise ValueError('origem/destino invalido')
    autoexec = soup_card / 'AUTOEXEC.BIN'
    if not autoexec.is_file():
        raise ValueError('AUTOEXEC.BIN original do Soup nao encontrado')
    original_se = soup_card / 'SE.BIN'
    if not original_se.is_file():
        raise ValueError('SE.BIN do Soup ausente: talvez nao seja o pacote 5DC')
    if not (soup_card / 'ML').is_dir():
        raise ValueError('ML/ ausente: use a pasta card/ completa do Soup')
    verified = inspect_legacy_loader(autoexec.read_bytes())
    metadata = json.loads((ROOT / 'dist/probe-manifest.json').read_text(encoding='utf-8'))
    data = probe.read_bytes()
    if metadata.get('target') != '5DC.111' or metadata.get('hardware_verified') is not False:
        raise ValueError('manifesto nao corresponde a prova experimental 5DC')
    if len(data) != metadata['size'] or hashlib.sha256(data).hexdigest() != metadata['sha256']:
        raise ValueError('payload Rebobina diferente do manifesto')
    if (data[:4] != b'FILM' or branch_target(data, 4) != metadata['rb_boot_offset'] or
            branch_target(data, 0x44) != metadata['rb_key_offset']):
        raise ValueError('cabecalho/entradas do payload Rebobina invalidos')
    parsed = {'size': len(data), 'sha256': metadata['sha256']}
    output.parent.mkdir(parents=True, exist_ok=True)
    shutil.copytree(soup_card, output)
    (output / 'SE.BIN').write_bytes(probe.read_bytes())
    manifest = {'warning': 'PROVA EXPERIMENTAL. NAO HOMOLOGADO EM CAMERA.',
                'base': verified, 'rebobina': parsed,
                'rollback': 'Copie o SE.BIN original do pacote Soup de volta.'}
    (output.parent / 'staging-manifest.json').write_text(
        json.dumps(manifest, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')
    return manifest


def main():
    ap = argparse.ArgumentParser(description='Cria SOMENTE dist/card-staging, sem escrever no cartao CF')
    ap.add_argument('--soup-card', type=Path, required=True,
                    help='pasta Canon-EOS-5D/card extraida do Soup Engine oficial')
    args = ap.parse_args()
    info = make_staging(args.soup_card, ROOT / 'dist/SE.BIN', STAGING)
    print('STAGING criado em:', STAGING)
    print('Manifesto:', ROOT / 'dist/staging-manifest.json')
    print('Hash do AUTOEXEC de origem (identificacao, nao homologacao):',
          info['base']['source_sha256'])
    print('AVISO: nao ha testes em camera real; nao usar cartao com fotos.')


if __name__ == '__main__':
    main()
