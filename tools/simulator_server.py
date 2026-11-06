#!/usr/bin/env python3
"""Servidor local, somente leitura, para o simulador visual conectado ao core C.

Nunca abre portas publicas, nao escreve em arquivos e nao comunica com a Canon.
Requer `make build/rebobina-sim-bridge` ou `make web-sim`.
"""
import argparse
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
import json
from pathlib import Path
import re
import subprocess
from urllib.parse import parse_qs, urlsplit

ROOT = Path(__file__).resolve().parents[1]
BRIDGE = ROOT / "build" / "rebobina-sim-bridge"
WEB = ROOT / "simulator" / "web"
MAX_EVENTS = 128
ALLOWED_ASSETS = {
    "/": (WEB / "index.html", "text/html; charset=utf-8"),
    "/index.html": (WEB / "index.html", "text/html; charset=utf-8"),
    "/app.css": (WEB / "app.css", "text/css; charset=utf-8"),
    "/app.js": (WEB / "app.js", "text/javascript; charset=utf-8"),
}


class SimError(Exception):
    pass


def parse_bridge(output):
    """Protocolo tabulado com textos que nao vem do usuario."""
    state = None
    rows = []
    for line in output.splitlines():
        parts = line.split("\t", 3)
        if line.startswith("STATE\t"):
            fields = line.split("\t")
            if state is not None or len(fields) != 5:
                raise SimError("Estado duplicado ou incompleto")
            try:
                opened, selected, page, active = map(int, fields[1:])
            except ValueError as exc:
                raise SimError("Estado nao numerico") from exc
            if opened not in (0, 1) or not 0 <= selected < 4 or not 0 <= page <= 4 or active not in (0, 1):
                raise SimError("Estado fora dos limites")
            state = {"opened": bool(opened), "selected": selected, "page": page,
                     "hello_active": bool(active)}
        elif line.startswith("ROW\t"):
            if len(parts) != 4:
                raise SimError("Linha invalida")
            try:
                row, selected = int(parts[1]), int(parts[2])
            except ValueError as exc:
                raise SimError("Linha nao numerica") from exc
            if not 0 <= row <= 8 or selected not in (0, 1) or len(parts[3]) > 250:
                raise SimError("Linha fora de limites")
            rows.append({"row": row, "selected": bool(selected), "text": parts[3]})
        else:
            raise SimError("Tipo de linha inesperado")
    if state is None or not 1 <= len(rows) <= 8:
        raise SimError("Resposta C incompleta")
    return {"state": state, "rows": rows, "source": "rebobina-core-c", "hardware": False}


def run_simulation(events):
    if not isinstance(events, str) or len(events) > MAX_EVENTS or re.fullmatch(r"[tudsb]*", events) is None:
        raise ValueError("Aceitos somente eventos t/u/d/s/b, no maximo 128")
    if not BRIDGE.is_file():
        raise SimError("Binario C ainda nao compilado; execute make web-sim")
    try:
        process = subprocess.run(
            [str(BRIDGE), events], cwd=str(ROOT), text=True, encoding="utf-8",
            stdout=subprocess.PIPE, stderr=subprocess.PIPE, timeout=3, check=False
        )
    except (OSError, subprocess.TimeoutExpired) as exc:
        raise SimError("Falha na execucao do nucleo C") from exc
    if process.returncode:
        raise SimError("Nucleo C terminou com erro")
    return parse_bridge(process.stdout)


class SimulatorHandler(BaseHTTPRequestHandler):
    server_version = "RebobinaHostSim/0.1"
    sys_version = ""

    def send_payload(self, status, data, content_type):
        self.send_response(status)
        self.send_header("Content-Type", content_type)
        self.send_header("Content-Length", str(len(data)))
        self.send_header("Cache-Control", "no-store")
        self.send_header("X-Content-Type-Options", "nosniff")
        self.send_header("X-Frame-Options", "DENY")
        self.send_header("Content-Security-Policy",
                         "default-src 'self'; script-src 'self'; style-src 'self'; "
                         "connect-src 'self'; object-src 'none'; base-uri 'none'; frame-ancestors 'none'")
        self.end_headers()
        self.wfile.write(data)

    def send_json(self, status, obj):
        data = json.dumps(obj, ensure_ascii=False, separators=(",", ":")).encode("utf-8")
        self.send_payload(status, data, "application/json; charset=utf-8")

    def do_GET(self):
        target = urlsplit(self.path)
        if target.path == "/api/state":
            params = parse_qs(target.query, keep_blank_values=True)
            if set(params) != {"events"} or len(params["events"]) != 1:
                self.send_json(400, {"error": "Forneca apenas um parametro events"})
                return
            try:
                result = run_simulation(params["events"][0])
            except ValueError as exc:
                self.send_json(400, {"error": str(exc)})
                return
            except SimError:
                self.send_json(503, {"error": "Nucleo C indisponivel"})
                return
            self.send_json(200, result)
            return
        if target.query or target.path not in ALLOWED_ASSETS:
            self.send_json(404, {"error": "Nao encontrado"})
            return
        path, content_type = ALLOWED_ASSETS[target.path]
        try:
            data = path.read_bytes()
        except OSError:
            self.send_json(503, {"error": "Arquivo de interface indisponivel"})
            return
        self.send_payload(200, data, content_type)

    def log_message(self, fmt, *args):
        return  # Evita logar dados enviados pelo usuario em stdout.

    def do_POST(self):
        self.send_json(405, {"error": "Servidor somente leitura"})


def main():
    parser = argparse.ArgumentParser(description="Simulador visual Rebobina (host, nao firmware)")
    parser.add_argument("--port", type=int, default=8765)
    args = parser.parse_args()
    if not 0 <= args.port <= 65535:
        parser.error("Porta deve estar entre 0 e 65535")
    server = ThreadingHTTPServer(("127.0.0.1", args.port), SimulatorHandler)
    print(f"Rebobina UI: http://127.0.0.1:{server.server_port}/", flush=True)
    print("Somente localhost. Nao conecta a camera nem grava no CF.", flush=True)
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        pass
    finally:
        server.server_close()


if __name__ == "__main__":
    main()
