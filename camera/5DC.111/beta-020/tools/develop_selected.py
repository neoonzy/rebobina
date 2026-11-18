#!/usr/bin/env python3
"""Apply the *selected* in-camera Rebobina profile to a CR2 on a LINUX HOST.

Requires the existing trusted rebobina-cr2-develop host executable built from
this repository and authentic Soup Engine .fs stock files. Never writes to CF,
RAW or input; delegates to the already-tested host developer and chooses a
new output name. This is NOT on-camera photographic processing.
"""
from pathlib import Path
import argparse
import subprocess
from config_reader import read_latest


def prepare(card: Path, raw: Path, output_dir: Path, tool: Path, stocks: Path):
    cfg = read_latest(card)
    if cfg is None:
        raise ValueError('Nenhum perfil RB8 salvo e valido no CF.')
    if not raw.is_file() or raw.suffix.lower() != '.cr2':
        raise ValueError('Entrada deve ser um arquivo .CR2 existente.')
    if not output_dir.is_dir():
        raise ValueError('Diretorio de saida deve existir.')
    if not tool.is_file():
        raise ValueError('rebobina-cr2-develop nao encontrado: compile no host primeiro.')
    stock = stocks / cfg['stock_file']
    if not stock.is_file():
        raise ValueError('Stock .fs nao encontrado (instale os stocks originais do Soup Engine).')
    stem = raw.stem
    if not stem.isascii() or not stem or len(stem) > 45 or not all(c.isalnum() or c in '-_' for c in stem):
        raise ValueError('Nome de entrada exige caracteres ASCII seguros.')
    dest = f'{stem}-RB-{cfg["film"]}.JPG'
    if (output_dir / dest).exists():
        raise ValueError('Nao sobrescrevo um JPEG de saida existente.')
    if output_dir.resolve() == raw.parent.resolve():
        raise ValueError('Diretorio de saida deve ser separado da pasta de origens.')
    return [str(tool), str(raw), str(stock), str(output_dir), dest, '2'], cfg


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--card', required=True, type=Path)
    ap.add_argument('--cr2', required=True, type=Path)
    ap.add_argument('--out', required=True, type=Path)
    ap.add_argument('--developer', required=True, type=Path, help='Linux rebobina-cr2-develop')
    ap.add_argument('--stocks', required=True, type=Path, help='Soup Engine stocks/*.fs directory')
    args = ap.parse_args()
    try:
        cmd, cfg = prepare(args.card,args.cr2,args.out,args.developer,args.stocks)
        print(f'Perfil selecionado na camera: {cfg["profile"]} | Rolo {cfg["roll"]}')
        cp = subprocess.run(cmd, check=False)
        return cp.returncode
    except (ValueError, OSError) as exc:
        print(f'Erro: {exc}')
        return 2

if __name__ == '__main__':
    raise SystemExit(main())
