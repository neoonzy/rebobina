#!/usr/bin/env python3
"""Compila e testa o motor upstream Soup Engine FIXADO por commit no Linux host.

Nao baixa firmware Canon, nao modifica cartao, nao realiza boot.
Copia de codigo GPLv2 upstream e mantida em build/soup-upstream (ignoravel).
"""
import argparse
from pathlib import Path
import shutil
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
URL = "https://github.com/jiijii0/soup-engine.git"
COMMIT = "6422c86b4530a5bdb4c5619f5287044ae7c661e3"
CHECKOUT = ROOT / "build" / "soup-upstream"


def execute(argv, *, cwd=None, timeout=180):
    print("+", " ".join(str(x) for x in argv), flush=True)
    return subprocess.run([str(x) for x in argv], cwd=cwd, timeout=timeout,
                          check=True)


def fetch_upstream():
    if (CHECKOUT / ".git").exists():
        raise RuntimeError("build/soup-upstream ja existe; nao sobrescrever fonte existente")
    CHECKOUT.mkdir(parents=True, exist_ok=False)
    execute(["git", "init", "--quiet", str(CHECKOUT)])
    execute(["git", "-C", str(CHECKOUT), "remote", "add", "origin", URL])
    execute(["git", "-C", str(CHECKOUT), "sparse-checkout", "init", "--cone"])
    execute(["git", "-C", str(CHECKOUT), "sparse-checkout", "set", "Canon-EOS-5D/src"])
    execute(["git", "-C", str(CHECKOUT), "fetch", "--depth=1", "origin", COMMIT], timeout=180)
    execute(["git", "-C", str(CHECKOUT), "checkout", "--detach", "FETCH_HEAD"])


def validate_source(source):
    src = source / "Canon-EOS-5D" / "src"
    if not (src / "engine" / "se.c").is_file() or not (src / "host" / "main.c").is_file():
        raise RuntimeError("pasta nao contem o source host do Soup Engine")
    # Requer um checkout Git para impedir usar inadvertidamente codigo de versao distinta.
    revision = subprocess.run(["git", "-C", str(source), "rev-parse", "HEAD"],
                              capture_output=True, text=True, check=True).stdout.strip()
    if revision != COMMIT:
        raise RuntimeError(f"versao de Soup Engine incorreta: {revision} != {COMMIT}")
    return src


def main():
    parser = argparse.ArgumentParser(description="Prova host: Soup Engine fixado + HAL Rebobina")
    parser.add_argument("--fetch", action="store_true",
                        help="baixa somente codigo-fonte da revisao fixada para build/")
    parser.add_argument("--source", type=Path,
                        help="checkout Git existente no commit fixado (sem download)")
    args = parser.parse_args()
    if args.fetch and args.source:
        parser.error("use --fetch ou --source, nao ambos")
    if shutil.which("cc") is None or shutil.which("git") is None:
        parser.error("cc e git sao necessarios")
    if args.fetch:
        fetch_upstream()
    src = validate_source(args.source.resolve() if args.source else CHECKOUT)
    print("Soup source fixado:", COMMIT, flush=True)
    execute(["make", "-C", str(src), "test"], timeout=240)
    out = ROOT / "build" / "soup-hal-smoke"
    execute([
        "cc", "-std=c11", "-O1", "-g", "-Wall", "-Wextra", "-Werror",
        "-DSE_HOSTED", "-I", str(ROOT / "include"), "-I", str(src / "engine"),
        str(ROOT / "tests" / "soup_hal_smoke.c"),
        str(ROOT / "src" / "host" / "hal_sim.c"),
        str(src / "engine" / "se.c"),
        "-o", str(out)
    ])
    execute([str(out)])
    print("OK: testes do motor Soup original + smoke de integracao HAL; "
          "SEM integracao no firmware/camera.", flush=True)


if __name__ == "__main__":
    try:
        main()
    except (subprocess.CalledProcessError, subprocess.TimeoutExpired,
            RuntimeError, OSError) as exc:
        print(f"ERRO de verificacao Soup host: {exc}", file=sys.stderr)
        sys.exit(1)
