"""File-migration regressions: preserve originals/payload, never overwrite outputs."""
import importlib.util
from pathlib import Path
import sys
import tempfile
import unittest
import xml.etree.ElementTree as ET

SPEC=importlib.util.spec_from_file_location('converter',Path(__file__).resolve().parents[1]/'tools/convert_mpc_presets.py')
C=importlib.util.module_from_spec(SPEC);sys.modules[SPEC.name]=C;SPEC.loader.exec_module(C)


def fixture(name='JC303 MPC',vendor='midilab',uid='4a634d33',quote='"'):
    return ('''<?xml version="1.0" encoding="UTF-8"?>\r
<pluginstate>\r
 <PLUGIN name=QNAMEQ descriptiveName="Public factory fixture"
 manufacturer=QVENDORQ uid=QUIDQ file=Q/old/plugin.soQ version="0.7.4.0"/>\r
 <preset>Test &amp; fixture</preset>\r
 <state>128.ABCxyz+/...012345</state>\r
</pluginstate>\r
'''.replace('QNAMEQ',quote+name+quote).replace('QVENDORQ',quote+vendor+quote).replace('QUIDQ',quote+uid+quote).replace('Q/old/plugin.soQ',quote+'/old/plugin.so'+quote)).encode()


class ConverterTests(unittest.TestCase):
    def setUp(self):
        self.temp=tempfile.TemporaryDirectory();self.root=Path(self.temp.name)
        self.source=self.root/'Input';self.source.mkdir();self.output=self.root/'Output'
    def tearDown(self):self.temp.cleanup()
    def put(self,raw=None,name='sound.xpl'):
        path=self.source/name;path.write_bytes(raw if raw is not None else fixture());return path
    def test_both_ports_payload_and_outside_header_unchanged(self):
        for name,vendor,uid in [('JC303 MPC','midilab','4a634d33'),('Geonkick MPC','OpenMPC','476b4d33')]:
            for quote in ['"',"'"]:
                original=b'\xef\xbb\xbf'+fixture(name,vendor,uid,quote)
                path=self.put(original);item=C.inspect(path,'/media/My & SD/VST2 Plugins')
                self.assertEqual(item.status,'ready',item.message)
                before=original.decode('utf-8-sig');after=item.data.decode('utf-8-sig')
                a=C.TAG.search(before);b=C.TAG.search(after)
                self.assertEqual(before[:a.start()],after[:b.start()]);self.assertEqual(before[a.end():],after[b.end():])
                old=ET.fromstring(original);new=ET.fromstring(item.data)
                self.assertEqual(old.findtext('state'),new.findtext('state'))
                self.assertEqual(new.find('PLUGIN').get('uid'),uid)
                self.assertEqual(new.find('PLUGIN').get('manufacturer'),'303inmyheart')
                self.assertIn('/media/My & SD/',new.find('PLUGIN').get('file'))
                self.assertEqual(path.read_bytes(),original)
    def test_preview_is_read_only_and_write_is_idempotent(self):
        path=self.put();original=path.read_bytes();items=C.plan(self.source,self.output)
        self.assertFalse(self.output.exists())
        self.assertEqual(C.apply(items,C.DEFAULT_PLUGIN_ROOT)['created'],1)
        self.assertEqual(C.apply(items,C.DEFAULT_PLUGIN_ROOT)['existing'],1)
        self.assertEqual(path.read_bytes(),original)
        self.assertEqual(len(list(self.output.rglob('*.xpl'))),1)
    def test_existing_output_is_never_overwritten(self):
        self.put();items=C.plan(self.source,self.output);p=items[0].destination
        p.parent.mkdir(parents=True);p.write_bytes(b'keep this existing output')
        C.apply(items,C.DEFAULT_PLUGIN_ROOT)
        self.assertEqual(p.read_bytes(),b'keep this existing output')
        self.assertTrue(p.with_name('sound (converted 1).xpl').exists())
    def test_changed_after_preview_is_not_written(self):
        path=self.put();items=C.plan(self.source,self.output);path.write_bytes(fixture().replace(b'128.ABC',b'128.DEF'))
        self.assertEqual(C.apply(items,C.DEFAULT_PLUGIN_ROOT)['error'],1)
        self.assertFalse(self.output.exists())
    def test_unrelated_and_unexpected_identity_are_skipped(self):
        for raw in [fixture(uid='deadbeef'),fixture(name='Another synth'),fixture(vendor='Unrelated')]:
            self.assertEqual(C.inspect(self.put(raw)).status,'skipped')
    def test_current_is_skipped(self):
        p=self.put();item=C.inspect(p);p.write_bytes(item.data)
        self.assertEqual(C.inspect(p).status,'current')
    def test_invalid_empty_entities_duplicate_tags(self):
        for raw in [b'<broken>',fixture().replace(b'128.ABCxyz+/...012345',b''),
                    b'<!DOCTYPE pluginstate>'+fixture(),fixture().replace(b' version=',b' uid="4a634d33" version='),
                    fixture().replace(b'<preset>',b'<!-- <PLUGIN/> --><preset>')]:
            self.assertEqual(C.inspect(self.put(raw)).status,'error')
    def test_source_output_equal_and_invalid_mpc_paths(self):
        self.put()
        with self.assertRaises(ValueError):C.plan(self.source,self.source)
        for path in ['C:\\VST2 Plugins','/nvme/../VST2 Plugins','/nvme/Synths','/nvme/VST2 Plugins\n']:
            with self.assertRaises(ValueError):C.plugin_root(path)
    def test_nested_output_is_excluded(self):
        self.put();inside=self.source/'Output';items=C.plan(self.source,inside);C.apply(items,C.DEFAULT_PLUGIN_ROOT)
        self.assertEqual(len(C.plan(self.source,inside)),1)
    def test_symlinks_are_not_followed_or_overwritten(self):
        path=self.put();link=self.source/'linked.xpl'
        try:link.symlink_to(path)
        except OSError:self.skipTest('Symlinks unavailable')
        self.assertEqual(C.inspect(link).status,'skipped')
        item=next(i for i in C.plan(self.source,self.output) if i.status=='ready');target=item.destination;target.parent.mkdir(parents=True)
        protected=self.root/'protected';protected.write_bytes(b'original');target.symlink_to(protected)
        C.apply([item],C.DEFAULT_PLUGIN_ROOT)
        self.assertEqual(protected.read_bytes(),b'original')
    def test_output_subfolder_cannot_escape_through_symlink(self):
        self.put();item=C.plan(self.source,self.output)[0]
        outside=self.root/'Outside';outside.mkdir();self.output.mkdir()
        try:(self.output/item.folder).symlink_to(outside,target_is_directory=True)
        except OSError:self.skipTest('Symlinks unavailable')
        with self.assertRaises(ValueError):C.apply([item],C.DEFAULT_PLUGIN_ROOT)
        self.assertEqual(list(outside.iterdir()),[])


if __name__=='__main__':unittest.main()
