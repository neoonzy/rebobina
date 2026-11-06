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


def linux_host_compat(src):
    """Patch estritamente no checkout TEMPORARIO em build/. Nao toca engine/."""
    if not sys.platform.startswith("linux"):
        return
    host = src / "host" / "main.c"
    original = host.read_text(encoding="utf-8")
    header = "#include <sys/mount.h>"
    anchor = "static int check_card(const char *root)"
    if original.count(header) != 1 or original.count(anchor) != 1:
        raise RuntimeError("source Soup mudou: patch host Linux nao e aplicavel")
    helper = """/* Compatibilidade Linux SOMENTE para o diagnostico checkcard no host.
 * A implementacao do motor em engine/ nao e alterada. */
static const char *rb_host_fs_name(const struct statfs *sfs)
{
    if (sfs->f_type == 0x4d44L) return "msdos";
    if (sfs->f_type == 0x2011BAB0L) return "exfat";
    return "unknown";
}
"""
    edited = original.replace(header, header + "\n#include <sys/vfs.h>\n#include <strings.h>")
    edited = edited.replace(anchor, helper + "\n" + anchor)
    edited = edited.replace("sfs.f_fstypename", "rb_host_fs_name(&sfs)")
    if edited.count("rb_host_fs_name(&sfs)") != 4:
        raise RuntimeError("quantidade de acessos ao tipo de filesystem mudou")
    host.write_text(edited, encoding="utf-8")
    print("Aplicado shim Linux apenas em host/main.c temporario, sem alterar engine/",
          flush=True)


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
    if args.fetch:
        linux_host_compat(src)
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
    jpeg_out = ROOT / "build" / "soup-jpeg-hal"
    execute([
        "cc", "-std=c11", "-O1", "-g", "-Wall", "-Wextra", "-Werror",
        "-DSE_HOSTED", "-I", str(ROOT / "include"), "-I", str(src / "engine"),
        str(ROOT / "tests" / "soup_jpeg_hal.c"),
        str(ROOT / "src" / "host" / "hal_sim.c"),
        str(ROOT / "src" / "core" / "output.c"),
        str(src / "engine" / "se.c"),
        str(src / "engine" / "jpeg_enc.c"),
        str(src / "engine" / "jpeg_dec.c"),
        "-lm", "-o", str(jpeg_out)
    ])
    execute([str(jpeg_out)])
    print("OK: testes do motor Soup original + smoke de integracao HAL; "
          "SEM integracao no firmware/camera.", flush=True)


if __name__ == "__main__":
    try:
        main()
    except (subprocess.CalledProcessError, subprocess.TimeoutExpired,
            RuntimeError, OSError) as exc:
        print(f"ERRO de verificacao Soup host: {exc}", file=sys.stderr)
        sys.exit(1)
