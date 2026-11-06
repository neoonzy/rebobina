"""Testes de ponta a ponta: servidor HTTP local -> bridge -> menu C."""
import json
from http.server import ThreadingHTTPServer
from pathlib import Path
import subprocess
import sys
import threading
import unittest
from urllib.error import HTTPError
from urllib.request import urlopen

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "tools"))
from simulator_server import SimulatorHandler, run_simulation, parse_bridge, SimError


class SimulatorTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.http = ThreadingHTTPServer(("127.0.0.1", 0), SimulatorHandler)
        cls.thread = threading.Thread(target=cls.http.serve_forever, daemon=True)
        cls.thread.start()
        cls.url = f"http://127.0.0.1:{cls.http.server_port}"

    @classmethod
    def tearDownClass(cls):
        cls.http.shutdown()
        cls.thread.join(timeout=3)
        cls.http.server_close()

    def api(self, suffix):
        with urlopen(self.url + suffix, timeout=5) as reply:
            return reply.status, json.load(reply)

    def test_closed_menu_is_core_c(self):
        status, data = self.api("/api/state?events=")
        self.assertEqual(status, 200)
        self.assertFalse(data["state"]["opened"])
        self.assertEqual(data["source"], "rebobina-core-c")
        self.assertFalse(data["hardware"])
        self.assertIn("TRASH", data["rows"][0]["text"])

    def test_menu_navigation_is_backed_by_c(self):
        _, opened = self.api("/api/state?events=t")
        self.assertTrue(opened["state"]["opened"])
        self.assertEqual(len(opened["rows"]), 5)
        _, selected = self.api("/api/state?events=tddu")
        self.assertEqual(selected["state"]["selected"], 1)
        self.assertTrue(selected["rows"][2]["selected"])

    def test_modules_lifecycle(self):
        _, page = self.api("/api/state?events=tdds")
        self.assertEqual(page["state"]["page"], 3)
        self.assertFalse(page["state"]["hello_active"])
        _, enabled = self.api("/api/state?events=tddss")
        self.assertTrue(enabled["state"]["hello_active"])
        self.assertIn("ATIVO", enabled["rows"][1]["text"])
        _, disabled = self.api("/api/state?events=tddsss")
        self.assertFalse(disabled["state"]["hello_active"])
        _, returned = self.api("/api/state?events=tddssb")
        self.assertEqual(returned["state"]["page"], 0)

    def test_shortcuts_invalid_and_limit(self):
        for events in ("x", "ts/", "t" * 129, "t%20b", "t;touch"):
            with self.subTest(events=events), self.assertRaises(HTTPError) as ctx:
                self.api("/api/state?events=" + events)
            self.assertEqual(ctx.exception.code, 400)

    def test_server_no_filesystem_access(self):
        for path in ("/README.md", "/../../etc/passwd", "/.git/config", "/tools/simulator_server.py"):
            with self.subTest(path=path), self.assertRaises(HTTPError) as ctx:
                self.api(path)
            self.assertEqual(ctx.exception.code, 404)

    def test_static_page_and_headers(self):
        with urlopen(self.url + "/", timeout=5) as response:
            page = response.read().decode("utf-8")
            self.assertEqual(response.status, 200)
            self.assertIn("Rebobina", page)
            self.assertIn("frame-ancestors 'none'", response.headers["Content-Security-Policy"])
        with urlopen(self.url + "/app.js", timeout=5) as response:
            self.assertIn("/api/state", response.read().decode("utf-8"))

    def test_post_is_rejected(self):
        from urllib.request import Request
        with self.assertRaises(HTTPError) as ctx:
            urlopen(Request(self.url + "/api/state", data=b"x"), timeout=5)
        self.assertEqual(ctx.exception.code, 405)

    def test_malformed_protocol_fails_closed(self):
        with self.assertRaises(SimError):
            parse_bridge("ROW\t0\t1\tNo state\n")
        with self.assertRaises(SimError):
            parse_bridge("STATE\t1\t0\t999\t0\nROW\t0\t0\tX\n")

    def test_cli_rejects_unknown_events(self):
        proc = subprocess.run([str(ROOT / "build/rebobina-sim-bridge"), "tx"],
                              text=True, capture_output=True, check=False)
        self.assertEqual(proc.returncode, 2)
        self.assertEqual(run_simulation("tdds")["state"]["page"], 3)


if __name__ == "__main__":
    unittest.main()
