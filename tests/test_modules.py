"""Testes host da CLI de manifestos (sem acesso a hardware)."""
import importlib.util
import json
import tempfile
import unittest
from pathlib import Path

SCRIPT = Path(__file__).resolve().parents[1] / "tools" / "modules.py"
SPEC = importlib.util.spec_from_file_location("rebobina_modules", SCRIPT)
modules = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(modules)


class ModuleToolTests(unittest.TestCase):
    def test_repository_manifests(self):
        records = modules.inspect_all(modules.DEFAULT_ROOT)
        self.assertEqual({entry["id"] for entry in records}, {"hello", "soup-engine"})

    def test_new_and_check(self):
        with tempfile.TemporaryDirectory() as path:
            root = Path(path) / "modules"
            created = modules.create_module(root, "meu-modulo", "Meu módulo")
            self.assertTrue((created / "module.json").exists())
            self.assertEqual(len(modules.inspect_all(root)), 1)
            with self.assertRaises(FileExistsError):
                modules.create_module(root, "meu-modulo", "Meu módulo")

    def test_reject_path_and_bad_api(self):
        with self.assertRaises(modules.ManifestError):
            modules.create_module(Path("/tmp/not-used-rebobina"), "../unsafe", "Inseguro")
        data = json.loads((modules.DEFAULT_ROOT / "hello" / "module.json").read_text())
        data["api_version"] = True
        with self.assertRaises(modules.ManifestError):
            modules.validate(data, "hello")

    def test_reject_mismatched_folder(self):
        data = json.loads((modules.DEFAULT_ROOT / "hello" / "module.json").read_text())
        with self.assertRaises(modules.ManifestError):
            modules.validate(data, "outra-pasta")


if __name__ == "__main__":
    unittest.main()
