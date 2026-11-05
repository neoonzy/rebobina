#!/usr/bin/env python3
"""Auditoria offline da pesquisa 5DC.111. NÃO prepara ou altera cartões/firmware."""
import argparse
import copy
import hashlib
import json
import re
import sys
from pathlib import Path

DEFAULT_INVENTORY = Path(__file__).resolve().parents[1] / "docs" / "data" / "5dc.111-evidence.json"
HEX32 = re.compile(r"0x[0-9A-Fa-f]{8}\Z")
SHA256 = re.compile(r"[0-9a-f]{64}\Z")
GIT_SHA = re.compile(r"[0-9a-f]{40}\Z")


class AuditError(ValueError):
    """A evidência não pode ser aceita como documento consistente."""


def require(condition, message):
    if not condition:
        raise AuditError(message)


def address(value, label):
    require(isinstance(value, str) and HEX32.fullmatch(value),
            f"{label}: requer endereço 32-bit 0xXXXXXXXX")
    return int(value, 16)


def validate_inventory(doc):
    require(isinstance(doc, dict), "inventário deve ser objeto JSON")
    require(doc.get("schema_version") == 1 and type(doc.get("schema_version")) is int,
            "schema_version deve ser 1")
    platform = doc.get("platform")
    require(isinstance(platform, dict), "platform ausente")
    require(platform.get("id") == "5DC.111", "somente 5DC.111 nesta auditoria")
    require(platform.get("canon_firmware") == "1.1.1", "firmware incompatível")
    require(platform.get("model") == "Canon EOS 5D Classic", "modelo incorreto")
    require(platform.get("os") == "VxWorks 5.5", "OS não corresponde à referência")
    require(isinstance(platform.get("soc"), str) and "ARM946E-S" in platform["soc"],
            "SoC/CPU de referência ausente")

    sources = doc.get("sources")
    require(isinstance(sources, dict) and {"soup", "modern_ml", "history"} <= sources.keys(),
            "fontes obrigatórias ausentes")
    for key in ("soup", "modern_ml"):
        source = sources[key]
        require(isinstance(source, dict), f"fonte {key} inválida")
        sha = source.get("commit")
        require(isinstance(sha, str) and GIT_SHA.fullmatch(sha), f"fonte {key}: commit inválido")
        url = source.get("url")
        require(isinstance(url, str) and url.startswith("https://github.com/")
                and sha in url, f"fonte {key}: URL sem revisão fixada")
    require(isinstance(sources["history"].get("url"), str) and
            sources["history"]["url"].startswith("https://"),
            "histórico sem URL")

    base = doc.get("base_binary")
    require(isinstance(base, dict), "base_binary ausente")
    require(base.get("source") == "soup", "origem do binário-base incorreta")
    require(type(base.get("expected_size_bytes")) is int and
            base["expected_size_bytes"] == 221184,
            "tamanho do executável-base não corresponde ao patch estudado")
    require(base.get("marker") == "gaonisoy" and base.get("marker_offset") == 4,
            "marcador do patch não corresponde à referência")
    sha = base.get("sha256")
    if sha is None:
        require(base.get("verification") == "not_identified",
                "binário sem hash deve permanecer como não identificado")
    else:
        require(isinstance(sha, str) and SHA256.fullmatch(sha),
                "sha256 deve ter 64 caracteres hexadecimais minúsculos")
        require(base.get("verification") == "identified",
                "hash preenchido requer verification=identified")

    hooks = doc.get("hooks")
    require(isinstance(hooks, list) and len(hooks) >= 5, "hooks insuficientes")
    names, locations = set(), set()
    for h in hooks:
        require(isinstance(h, dict), "entrada de hook inválida")
        label = h.get("name")
        require(isinstance(label, str) and label not in names, "hook sem nome único")
        names.add(label)
        loc = address(h.get("address"), label)
        require(loc not in locations, f"hook duplicado: {label}")
        locations.add(loc)
        require(0x00980000 <= loc < 0x009B5000 and loc % 4 == 0,
                f"{label}: fora da imagem relocada relatada")
        require(h.get("scope") == "relocated-ml-2012" and h.get("source") == "soup",
                f"{label}: origem/escopo incorretos")
        require(isinstance(h.get("path"), str) and h["path"].startswith("Canon-EOS-5D/"),
                f"{label}: arquivo de referência ausente")

    roms = doc.get("rom_functions")
    require(isinstance(roms, list) and len(roms) >= 6, "serviços da ROM insuficientes")
    rom_names = set()
    for f in roms:
        require(isinstance(f, dict), "função ROM inválida")
        name = f.get("name")
        require(isinstance(name, str) and name not in rom_names, "função ROM duplicada")
        rom_names.add(name)
        loc = address(f.get("address"), name)
        require(loc >= 0xFF800000 and loc % 4 == 0, f"{name}: endereço ROM inesperado")
        require(f.get("source") == "soup" and
                f.get("path") == "Canon-EOS-5D/src/ml/payload/cam.h",
                f"{name}: fonte inválida")

    regions = doc.get("regions")
    require(isinstance(regions, list) and len(regions) == 3,
            "esperadas três regiões de referência do linker Soup")
    used = []
    region_names = set()
    for reg in regions:
        require(isinstance(reg, dict), "região inválida")
        name = reg.get("name")
        require(isinstance(name, str) and name not in region_names,
                "região sem nome único")
        region_names.add(name)
        start = address(reg.get("start"), name)
        length = reg.get("length_bytes")
        require(type(length) is int and 0 < length <= 131072 and length % 4 == 0,
                f"{name}: tamanho inválido")
        stop = start + length
        require(0x009D0000 <= start < stop <= 0x00A00000,
                f"{name}: fora dos limites relatados pelo Soup")
        require(reg.get("source") == "soup" and
                reg.get("path") == "Canon-EOS-5D/src/ml/payload/payload.ld",
                f"{name}: origem inválida")
        for earlier_start, earlier_stop in used:
            require(stop <= earlier_start or start >= earlier_stop,
                    f"{name}: sobreposição de regiões")
        used.append((start, stop))
    require(region_names == {"soup_payload", "soup_menu_stack", "soup_worker_stack"},
            "nomes das regiões inesperados")
    require(isinstance(doc.get("certainty"), str) and
            "NOT verified" in doc["certainty"], "ressalva de validação ausente")
    return {"sources": len(sources), "hooks": len(hooks), "rom_functions": len(roms),
            "regions": len(regions), "base_sha_known": sha is not None}


def load_inventory(path=DEFAULT_INVENTORY):
    try:
        doc = json.loads(Path(path).read_text(encoding="utf-8"))
    except (OSError, UnicodeError, json.JSONDecodeError) as exc:
        raise AuditError(f"não foi possível ler inventário: {exc}") from exc
    return doc


def inspect_autoexec_bytes(data, expected_sha256, base):
    """Inspeção estrutural + hash, somente em bytes de entrada; NÃO valida boot."""
    require(isinstance(expected_sha256, str) and SHA256.fullmatch(expected_sha256),
            "é obrigatório SHA-256 explícito (64 hex minúsculos)")
    require(isinstance(data, bytes), "arquivo deve ser bytes")
    require(len(data) == base["expected_size_bytes"], "tamanho do AUTOEXEC não corresponde")
    off, marker = base["marker_offset"], base["marker"].encode("ascii")
    require(data[off:off + len(marker)] == marker, "marcador do AUTOEXEC incorreto")
    actual = hashlib.sha256(data).hexdigest()
    require(actual == expected_sha256, "SHA-256 do AUTOEXEC diferente do valor informado")
    return {"sha256": actual, "size": len(data), "marker": base["marker"],
            "result": "apenas inspeção estrutural e igualdade de hash; sem homologação em câmera"}


def inspect_file(path, expected_sha256, base):
    """Não abre para escrita, não prepara um CF e recusa tamanhos inesperados."""
    try:
        p = Path(path)
        size = p.stat().st_size
        require(size == base["expected_size_bytes"], "tamanho do AUTOEXEC não corresponde")
        return inspect_autoexec_bytes(p.read_bytes(), expected_sha256, base)
    except OSError as exc:
        raise AuditError(f"leitura do arquivo falhou: {exc}") from exc


def main(argv=None):
    parser = argparse.ArgumentParser(description="Pesquisa 5D Classic: somente leitura, SEM instalar firmware")
    parser.add_argument("--inventory", type=Path, default=DEFAULT_INVENTORY)
    sub = parser.add_subparsers(dest="cmd", required=True)
    sub.add_parser("check", help="valida consistência do inventário offline")
    inspect = sub.add_parser("inspect-autoexec",
                             help="compara arquivo local ao SHA-256 fornecido; NÃO homologa a câmera")
    inspect.add_argument("--file", required=True, type=Path)
    inspect.add_argument("--expected-sha256", required=True)
    args = parser.parse_args(argv)
    try:
        doc = load_inventory(args.inventory)
        summary = validate_inventory(doc)
        if args.cmd == "check":
            print("OK: inventário consistente (somente documentação):", json.dumps(summary))
        else:
            result = inspect_file(args.file, args.expected_sha256, doc["base_binary"])
            print(json.dumps(result, ensure_ascii=False, indent=2))
    except AuditError as exc:
        print(f"FALHA: {exc}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
