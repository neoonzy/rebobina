#!/usr/bin/env python3
"""Compila e testa o motor upstream Soup Engine FIXADO por commit no Linux host.

Nao baixa firmware Canon, nao modifica cartao, nao realiza boot.
Copia de codigo GPLv2 upstream e mantida em build/soup-upstream (ignoravel).
"""
import argparse
from pathlib import Path
import hashlib
import shutil
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[1]
URL = "https://github.com/jiijii0/soup-engine.git"
COMMIT = "6422c86b4530a5bdb4c5619f5287044ae7c661e3"
CHECKOUT = ROOT / "build" / "soup-upstream"


def execute(argv, *, cwd=None, timeout=180):
    print("+", " ".join(str(x) for x in argv), flush=True)
    return subprocess.run([str(x) for x in argv], cwd=cwd, timeout=timeout,
                          check=True)


def capture_output(argv):
    """Executa e publica stdout de teste, devolvendo texto para comparacao."""
    print("+", " ".join(str(x) for x in argv), flush=True)
    result = subprocess.run([str(x) for x in argv], stdout=subprocess.PIPE,
                            stderr=None, text=True, timeout=180, check=True)
    print(result.stdout, end="", flush=True)
    return result.stdout


def sanitizer_encoder_copy(src):
    """Corrige UB de shift negativo SOMENTE numa copia para sanitizadores.

    O codigo GPLv2 upstream em engine/jpeg_enc.c permanece intacto.
    """
    original = (src / "engine" / "jpeg_enc.c").read_text(encoding="utf-8")
    fixes = (
        ("p[0] = (tmp10 + tmp11) << PASS1_BITS;",
         "p[0] = (tmp10 + tmp11) * (1 << PASS1_BITS);"),
        ("p[4] = (tmp10 - tmp11) << PASS1_BITS;",
         "p[4] = (tmp10 - tmp11) * (1 << PASS1_BITS);"),
    )
    for old, new in fixes:
        if original.count(old) != 1:
            raise RuntimeError("o codigo JPEG do Soup mudou: correcao nao aplicavel")
        original = original.replace(old, new)
    folder = ROOT / "build" / "soup-sanitizer-compat"
    folder.mkdir(parents=True, exist_ok=True)
    out = folder / "jpeg_enc.c"
    out.write_text(original, encoding="utf-8")
    print("Ponte de sanitizador: jpeg_enc.c copiado com 2 shifts negativos substituidos "
          "por multiplicacao. Engine upstream intacto.", flush=True)
    return out


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
    stock_out = ROOT / "build" / "soup-stock-output"
    # O GCC 15 emite -Wstringop-overflow na copia de borda do halo em
    # look.c. Mantemos o aviso visivel sem alterar o upstream GPLv2.
    execute([
        "cc", "-std=c11", "-O1", "-g", "-Wall", "-Wextra", "-Werror",
        "-Wno-error=stringop-overflow",
        "-DSE_HOSTED", "-I", str(ROOT / "include"), "-I", str(src / "engine"),
        str(ROOT / "tests" / "soup_stock_output.c"),
        str(ROOT / "src" / "host" / "hal_sim.c"),
        str(ROOT / "src" / "core" / "output.c"),
        str(src / "engine" / "se.c"),
        str(src / "engine" / "jpeg_enc.c"),
        str(src / "engine" / "jpeg_dec.c"),
        str(src / "engine" / "stock.c"),
        str(src / "engine" / "look.c"),
        str(src / "engine" / "grade.c"),
        str(src / "engine" / "lut.c"),
        str(src / "engine" / "wb.c"),
        "-lm", "-o", str(stock_out)
    ])
    stocks_to_test = ("soup800.fs", "portra.fs", "acros.fs")
    original_outputs = {
        stock: capture_output([str(stock_out), str(src / "stocks" / stock)])
        for stock in stocks_to_test
    }
    # UBSan encontrou shift negativo em jpeg_enc.c:353; testamos o ajuste
    # so numa copia em build/, sem alterar codigo do motor de referencia.
    if not shutil.which("clang"):
        raise RuntimeError("clang requerido para ASan/UBSan do processamento Soup")
    compatible_encoder = sanitizer_encoder_copy(src)
    san_out = ROOT / "build" / "soup-stock-sanitized"
    execute([
        "clang", "-std=c11", "-O1", "-g", "-Wall", "-Wextra",
        "-fsanitize=address,undefined", "-fno-sanitize-recover=all",
        "-DSE_HOSTED", "-I", str(ROOT / "include"), "-I", str(src / "engine"),
        str(ROOT / "tests" / "soup_stock_output.c"),
        str(ROOT / "src" / "host" / "hal_sim.c"),
        str(ROOT / "src" / "core" / "output.c"),
        str(src / "engine" / "se.c"),
        str(compatible_encoder),
        str(src / "engine" / "jpeg_dec.c"),
        str(src / "engine" / "stock.c"),
        str(src / "engine" / "look.c"),
        str(src / "engine" / "grade.c"),
        str(src / "engine" / "lut.c"),
        str(src / "engine" / "wb.c"),
        "-lm", "-o", str(san_out)
    ])
    for stock in stocks_to_test:
        actual = capture_output([str(san_out), str(src / "stocks" / stock)])
        if actual != original_outputs[stock]:
            raise RuntimeError(f"output JPEG do stock {stock} divergiu com o patch de UB")
    print("OK: 3 perfis Soup em builds normal + ASan/UBSan, resultados identicos.", flush=True)
    # CR2 host: header TIFF sintetico contem JPEG feito pelo proprio Soup.
    cr2_out = ROOT / "build" / "soup-cr2-preview"
    cr2_sources = [
        str(src / "engine" / "se.c"),
        str(src / "engine" / "cr2.c"),
        str(src / "engine" / "wb.c"),
        str(src / "engine" / "jpeg_dec.c"),
        str(src / "engine" / "jpeg_enc.c"),
    ]
    execute([
        "cc", "-std=c11", "-O1", "-g", "-Wall", "-Wextra", "-Werror",
        "-DSE_HOSTED", "-I", str(ROOT / "include"), "-I", str(src / "engine"),
        str(ROOT / "tests" / "soup_cr2_preview.c"),
        str(ROOT / "src" / "host" / "cr2_host.c"),
        *cr2_sources, "-lm", "-o", str(cr2_out)
    ])
    execute([str(cr2_out)])
    inspect_out = ROOT / "build" / "rebobina-cr2-inspect"
    execute([
        "cc", "-std=c11", "-O1", "-g", "-Wall", "-Wextra", "-Werror",
        "-DSE_HOSTED", "-I", str(ROOT / "include"), "-I", str(src / "engine"),
        str(ROOT / "src" / "host" / "cr2_inspect.c"),
        str(ROOT / "src" / "host" / "cr2_host.c"),
        *cr2_sources, "-lm", "-o", str(inspect_out)
    ])
    # CLI nao modifica arquivos; um argumento inválido e rejeitado.
    result = subprocess.run([str(inspect_out)], capture_output=True, text=True)
    if result.returncode != 2:
        raise RuntimeError("cr2-inspect nao rejeitou chamada sem arquivo")
    print("OK: CR2 header sintetico + CLI de inspecao somente leitura.", flush=True)
    # Caminho inteiro do upstream: envelope TIFF/CR2 artificial -> preview
    # JPEG -> Soup 800 -> JPEG de saida. NAO e uma foto Canon/RAW verdadeira.
    execute(["make", "-C", str(src), "out/previewcheck"], timeout=120)
    with tempfile.TemporaryDirectory(prefix="rb-synthetic-cr2-", dir=str(ROOT / "build")) as tmp:
        sample = Path(tmp) / "synthetic-no-raw.CR2"
        developed = Path(tmp) / "soup800.jpg"
        execute([str(cr2_out), str(sample)])
        original_sha = hashlib.sha256(sample.read_bytes()).hexdigest()
        execute([str(inspect_out), str(sample)])
        execute([
            str(src / "out" / "previewcheck"), str(sample),
            str(src / "stocks" / "soup800.fs"), "1", str(developed)
        ])
        jpeg = developed.read_bytes()
        if len(jpeg) <= 100 or not jpeg.startswith(bytes((0xff, 0xd8))) or not jpeg.endswith(bytes((0xff, 0xd9))):
            raise RuntimeError("previewcheck nao produziu JPEG sintético reconhecivel")
        if hashlib.sha256(sample.read_bytes()).hexdigest() != original_sha:
            raise RuntimeError("previewcheck ou cr2-inspect alteraram a entrada")
        print("OK: envelope CR2 sintetico -> preview -> Soup 800 -> JPEG",
              f"({len(jpeg)} bytes, arquivo de entrada inalterado)", flush=True)
    print("OK: testes do motor Soup original + smoke de integracao HAL; "
          "SEM integracao no firmware/camera.", flush=True)


if __name__ == "__main__":
    try:
        main()
    except (subprocess.CalledProcessError, subprocess.TimeoutExpired,
            RuntimeError, OSError) as exc:
        print(f"ERRO de verificacao Soup host: {exc}", file=sys.stderr)
        sys.exit(1)
