"""Static safety checks; do not claim that these exercise actual CF writes."""
from pathlib import Path
import unittest

ROOT=Path(__file__).resolve().parents[1]

class InstallerGuards(unittest.TestCase):
    def test_explicit_apply_backup_and_hash(self):
        ins=(ROOT/'tools/install-rebobina.ps1').read_text(encoding='utf-8')
        restore=(ROOT/'tools/restore-rebobina.ps1').read_text(encoding='utf-8')
        for token in ('[switch]$Apply','INSTALAR','SE-ANTERIOR.BIN','Get-FileHash','prepare-test.ps1','if(-not $Apply)'):
            self.assertIn(token,ins)
        for token in ('[switch]$Apply','RESTAURAR','SE-ANTERIOR.BIN','Get-FileHash','if(-not $Apply)'):
            self.assertIn(token,restore)
        for source in (ins,restore):
            self.assertNotIn('Format-Volume',source)
            self.assertNotIn('Clear-Disk',source)
            self.assertNotIn('Set-Disk',source)
    def test_legacy_config_and_profile_mapping(self):
        data=(ROOT/'tools/config_reader.py').read_text(encoding='utf-8')
        self.assertIn('RB8CFG',data)
        self.assertIn('velvia.fs',data)
        self.assertIn('eterna.fs',data)
if __name__=='__main__':
    unittest.main()
