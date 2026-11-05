#!/usr/bin/env python3
"""Ferramenta host para manifestos Rebobina. Não acessa câmera nem cartão."""
import argparse
import json
import re
import sys
from pathlib import Path

DEFAULT_ROOT = Path(__file__).resolve().parents[1] / "modules"
ID = re.compile(r"[a-z][a-z0-9-]{0,47}\Z")
VERSION = re.compile(r"\d+\.\d+\.\d+\Z")
CAPABILITY = re.compile(r"[a-z][a-z0-9-]{0,47}\Z")
STATUSES = {"planned", "prototype", "camera-tested"}
PLATFORMS = {"host", "5DC.111"}
REQUIRED = {"id", "name", "version", "api_version", "status",
            "platforms", "capabilities", "description"}
OPTIONAL = {"source"}


class ManifestError(Exception):
    pass


def validate(data, directory=None):
    """Devolve um manifesto validado ou dispara ManifestError."""
    if not isinstance(data, dict):
        raise ManifestError("o manifesto deve ser um objeto JSON")
    missing = REQUIRED - data.keys()
    unknown = data.keys() - REQUIRED - OPTIONAL
    if missing:
        raise ManifestError("campos ausentes: " + ", ".join(sorted(missing)))
    if unknown:
        raise ManifestError("campos desconhecidos: " + ", ".join(sorted(unknown)))
    ident = data["id"]
    if not isinstance(ident, str) or not ID.fullmatch(ident):
        raise ManifestError("id inválido: use letras minúsculas, dígitos e hífens")
    if directory is not None and ident != directory:
        raise ManifestError(f"id '{ident}' difere do diretório '{directory}'")
    for key in ("name", "description"):
        if not isinstance(data[key], str) or not data[key].strip():
            raise ManifestError(f"campo '{key}' deve ser texto não vazio")
    if not isinstance(data["version"], str) or not VERSION.fullmatch(data["version"]):
        raise ManifestError("version deve seguir MAJOR.MINOR.PATCH")
    if type(data["api_version"]) is not int or data["api_version"] != 1:
        raise ManifestError("api_version deve ser o inteiro 1")
    if data["status"] not in STATUSES:
        raise ManifestError("status inválido")
    platforms = data["platforms"]
    if (not isinstance(platforms, list) or not platforms or
        any(not isinstance(x, str) or x not in PLATFORMS for x in platforms) or
        len(platforms) != len(set(platforms))):
        raise ManifestError("platforms inválidas ou duplicadas")
    caps = data["capabilities"]
    if (not isinstance(caps, list) or
        any(not isinstance(x, str) or not CAPABILITY.fullmatch(x) for x in caps) or
        len(caps) != len(set(caps))):
        raise ManifestError("capabilities inválidas ou duplicadas")
    if "source" in data and (not isinstance(data["source"], str) or
                             not data["source"].startswith("https://")):
        raise ManifestError("source deve começar com https://")
    return data


def inspect_all(root):
    if not root.is_dir():
        raise ManifestError(f"pasta de módulos não encontrada: {root}")
    manifests = []
    for folder in sorted(root.iterdir()):
        if folder.is_symlink():
            raise ManifestError(f"link simbólico não permitido: {folder}")
        if not folder.is_dir():
            continue
        path = folder / "module.json"
        if path.is_symlink() or not path.is_file():
            raise ManifestError(f"manifesto ausente ou inválido: {path}")
        try:
            data = json.loads(path.read_text(encoding="utf-8"))
        except (OSError, UnicodeError, json.JSONDecodeError) as exc:
            raise ManifestError(f"não foi possível ler {path}: {exc}") from exc
        try:
            validate(data, folder.name)
        except ManifestError as exc:
            raise ManifestError(f"{path}: {exc}") from exc
        manifests.append(data)
    if not manifests:
        raise ManifestError("nenhum módulo encontrado")
    return manifests


def create_module(root, ident, name):
    validate({"id": ident, "name": name, "version": "0.1.0",
              "api_version": 1, "status": "planned", "platforms": ["host"],
              "capabilities": [], "description": "Rascunho de módulo."})
    target = root / ident
    root.mkdir(parents=True, exist_ok=True)
    target.mkdir(exist_ok=False)
    data = {"id": ident, "name": name, "version": "0.1.0",
            "api_version": 1, "status": "planned", "platforms": ["host"],
            "capabilities": [], "description": "Rascunho de módulo."}
    (target / "module.json").write_text(
        json.dumps(data, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    (target / "README.md").write_text(
        f"# {name}\n\nMódulo em planejamento. Não executável na câmera.\n",
        encoding="utf-8")
    return target


def main(argv=None):
    parser = argparse.ArgumentParser(
        description="Gerencia manifestos de desenvolvimento (não instala firmware).")
    parser.add_argument("--root", type=Path, default=DEFAULT_ROOT,
                        help="diretório com módulos")
    sub = parser.add_subparsers(dest="command", required=True)
    sub.add_parser("check", help="valida os manifestos")
    sub.add_parser("list", help="lista os manifestos")
    new = sub.add_parser("new", help="cria um esqueleto de módulo")
    new.add_argument("id")
    new.add_argument("--name", required=True)
    args = parser.parse_args(argv)
    try:
        if args.command == "new":
            path = create_module(args.root, args.id, args.name)
            print(f"criado: {path} (somente metadados)")
        else:
            modules = inspect_all(args.root)
            if args.command == "check":
                print(f"OK: {len(modules)} manifestos válidos (sem validar hardware)")
            else:
                for module in modules:
                    print(f"{module['id']}: {module['name']} "
                          f"[{module['status']}] ({', '.join(module['platforms'])})")
    except (ManifestError, FileExistsError, OSError) as exc:
        print(f"erro: {exc}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
