#!/usr/bin/env python3
"""Read Rebobina 0.3.0-beta diagnostic reports on a computer (read only)."""
from argparse import ArgumentParser
from pathlib import Path
from config_reader import read_latest


def main():
    ap=ArgumentParser(description=__doc__)
    ap.add_argument('card',type=Path,help='CF root, e.g. D:\\ or /media/CF')
    a=ap.parse_args()
    for name in ('RB9BOOT.TXT','RB9TEST.TXT','RB9SESSION.TXT'):
        f=a.card/name
        print(f'\n--- {name} ---')
        if f.is_file():print(f.read_text(encoding='ascii',errors='replace'))
        else:print('Ainda nao gerado (SESSION exige SALVAR/EXPORTAR).')
    print('\n--- FILM / CADERNO ---')
    cfg=read_latest(a.card)
    if cfg:
        for key in ('profile','roll','frames','sequence','slot','modules'):
            print(f'{key}: {cfg[key]}')
    else:print('Nenhuma configuracao beta valida salva ainda.')
    print('Leitura apenas. Fotos e cartao nao foram modificados.')

if __name__=='__main__':main()
