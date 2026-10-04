import importlib.util
from pathlib import Path
import sys
import tempfile
import unittest

root=Path(__file__).resolve().parent
spec=importlib.util.spec_from_file_location('decomp_alias_preview',root/'decomp_manifest.py')
manifest=importlib.util.module_from_spec(spec)
sys.modules[spec.name]=manifest
spec.loader.exec_module(manifest)

class AliasManifestTests(unittest.TestCase):
    def setUp(self):
        self.temp=tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.path=Path(self.temp.name)
        (self.path/'metadata').mkdir()
        (self.path/'cfg').mkdir()
        (self.path/'metadata/functions.toml').write_text('''format = 1
[[function]]
address = "00:057D"
name = "Lufia2RamBlockMove"
source = "src/system/block_move.c"
entry_mx = "M0X0"
exit_mx = "M0X0"
status = "verified"
''')
        for bank in ['83','86']:
            (self.path/f'cfg/bank{bank}.cfg').write_text(f'bank = {bank}\n')
    def binding(self,extra='dispatch_addresses = ["83:057D", "86:057D"]'):
        (self.path/'bindings.toml').write_text('''format = 1
[[binding]]
address = "00:057D"
bridge = "Lufia2DecompBridge_00057D"
'''+extra+'\n')
    def generate(self,**kwargs):
        return manifest.generate(self.path,self.path/'bindings.toml',self.path/'cfg',
                                 self.path/'out',self.path/'report.json',**kwargs)
    def test_two_execution_banks_one_semantic_function(self):
        self.binding()
        report=self.generate()
        self.assertEqual(len(report['selected']),1)
        self.assertEqual(report['selected'][0]['address'],'00:057D')
        self.assertEqual(report['selected'][0]['dispatch_addresses'],['83:057D','86:057D'])
        for bank in ['83','86']:
            self.assertIn('hle_func 057D Lufia2DecompBridge_00057D',(self.path/f'out/bank{bank}.cfg').read_text())
    def test_default_still_requires_declared_bank(self):
        self.binding('')
        with self.assertRaisesRegex(manifest.ManifestError,'no source cfg declares bank 00'):self.generate()
    def test_draft_stays_unselected(self):
        path=self.path/'metadata/functions.toml'
        path.write_text(path.read_text().replace('"verified"','"draft"'))
        self.binding()
        self.assertEqual(self.generate()['selected'],[])
    def test_reference_only_generates_no_overrides(self):
        self.binding()
        self.assertEqual(self.generate(reference_only=True)['selected'],[])
        self.assertNotIn('hle_func',(self.path/'out/bank83.cfg').read_text())
    def test_duplicate_normalized_alias_rejected(self):
        self.binding('dispatch_addresses = ["83:057d", "83:057D"]')
        with self.assertRaisesRegex(manifest.ManifestError,'duplicate dispatch address'):self.generate()
    def test_different_pc_rejected(self):
        self.binding('dispatch_addresses = ["83:057E"]')
        with self.assertRaisesRegex(manifest.ManifestError,'retain the entry PC'):self.generate()
    def test_empty_aliases_rejected(self):
        self.binding('dispatch_addresses = []')
        with self.assertRaisesRegex(manifest.ManifestError,'nonempty array'):self.generate()
    def test_scalar_alias_rejected(self):
        self.binding('dispatch_addresses = "83:057D"')
        with self.assertRaisesRegex(manifest.ManifestError,'nonempty array'):self.generate()
    def test_alias_cannot_replace_another_semantic_function(self):
        path=self.path/'metadata/functions.toml'
        path.write_text(path.read_text()+path.read_text().split('[[function]]')[1].join(['\n[[function]]','']).replace('00:057D','86:057D').replace('Lufia2RamBlockMove','OtherFunction'))
        self.binding()
        with self.assertRaisesRegex(manifest.ManifestError,'another semantic function'):self.generate()
    def test_hand_authored_override_collision(self):
        (self.path/'cfg/bank83.cfg').write_text('bank = 83\nhle_func 057D ExistingBridge\n')
        self.binding()
        with self.assertRaisesRegex(manifest.ManifestError,'hand-authored hle_func'):self.generate()
    def test_fallback_parser_string_array(self):
        self.binding()
        text=(self.path/'bindings.toml').read_text()
        parsed=manifest._load_basic_toml(text,self.path/'bindings.toml')
        self.assertEqual(parsed['binding'][0]['dispatch_addresses'],['83:057D','86:057D'])

if __name__=='__main__':unittest.main(verbosity=2)
