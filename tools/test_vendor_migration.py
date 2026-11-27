#!/usr/bin/env python3
"""Exercise old/new release install scripts in an isolated desktop fixture.
No device access. Uses MPC_INSTALL_TEST, no real settings or service changes.
Usage: test_vendor_migration.py OLD.zip NEW.zip
"""
import hashlib,json,os,subprocess,sys,tempfile,zipfile
from pathlib import Path
from xml.etree import ElementTree as ET
old_zip,new_zip=map(lambda s:Path(s).resolve(),sys.argv[1:])
def unpack(archive,dest):
 with zipfile.ZipFile(archive) as z:
  for n in z.namelist():assert not n.startswith('/') and '..' not in Path(n).parts
  z.extractall(dest)
 top=next(dest.iterdir());return top,json.loads((top/'mpc-plugin.json').read_text())
def run(top,script,env,target):
 r=subprocess.run(['sh',script,'-y','-n','-t',str(target)],cwd=top,env=env,text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
 assert r.returncode==0,r.stdout
 return r.stdout
for foreign in (False,True):
 with tempfile.TemporaryDirectory(prefix='mpc-vendor-migration-') as scratch:
  scratch=Path(scratch);target=scratch/'VST2 Plugins';target.mkdir()
  a,old=unpack(old_zip,scratch/'old-package');b,new=unpack(new_zip,scratch/'new-package')
  assert old['id']==new['id'] and old['so']==new['so']
  assert old['skin'] in new['retire'] and new['skin'].startswith('303inmyheart - VST - ')
  settings=scratch/'MPC.settings';settings.write_text('<PROPERTIES>\n<VALUE name="SynthContentLocations"><Location>'+str(target)+'</Location></VALUE>\n<VALUE name="pluginList-arm-64bit"><KNOWNPLUGINS>\n<PLUGIN name="Unrelated" file="unrelated.so" uid="deadbeef" manufacturer="Other" />\n</KNOWNPLUGINS></VALUE>\n</PROPERTIES>')
  env=dict(os.environ,MPC_INSTALL_TEST='1',MPC_SETTINGS=str(settings),MPC_TEST_LOG=str(scratch/'service-calls'),MPC_STORE_TMP=str(scratch))
  run(a,'install.sh',env,target)
  old_dir=target/old['skin'];new_dir=target/new['skin'];assert old_dir.is_dir()
  # Normal MPC user presets/projects live outside the installed plugin folder.
  user=scratch/'MPC Documents'/'Presets';user.mkdir(parents=True)
  user_preset=user/'user-sound.xpl';user_preset.write_bytes(b'user preset sentinel; stable plugin UID')
  declared={}
  for rel in new.get('user_data',[]):
   path=old_dir/rel;assert not path.exists(),'Fixture expects unshipped user setting'
   path.parent.mkdir(parents=True,exist_ok=True);declared[rel]=b'/fixture/sample-library\n';path.write_bytes(declared[rel])
  old_uid=ET.parse(old_dir/'plugin-meta.xml').getroot().get('uid')
  if foreign:
   meta=json.loads((old_dir/'mpc-plugin.json').read_text());meta['id']='another-plugin';(old_dir/'mpc-plugin.json').write_text(json.dumps(meta));(old_dir/'foreign-file').write_bytes(b'do not remove')
  run(b,'install.sh',env,target)
  assert new_dir.is_dir()
  if foreign:assert (old_dir/'foreign-file').read_bytes()==b'do not remove'
  else:
   assert not old_dir.exists(),'Old matching-id folder must be retired'
   for rel,data in declared.items():assert (new_dir/rel).read_bytes()==data
  metadata=ET.parse(new_dir/'plugin-meta.xml').getroot();assert metadata.get('uid')==old_uid
  assert metadata.get('manufacturer')=='303inmyheart'
  entries=list(ET.parse(settings).iter('PLUGIN'));ours=[e for e in entries if e.get('uid')==old_uid]
  assert len(ours)==1 and ours[0].get('manufacturer')=='303inmyheart'
  assert ours[0].get('file')==str(new_dir/new['so'])
  assert len([e for e in entries if e.get('uid')=='deadbeef'])==1
  for p in (new_dir/'Presets').glob('*.xpl'):
   plugin=ET.parse(p).find('PLUGIN');assert plugin.get('manufacturer')=='303inmyheart'
   assert plugin.get('uid')==old_uid and plugin.get('file')==str(new_dir/new['so'])
  run(b,'install.sh',env,target)
  assert len([e for e in ET.parse(settings).iter('PLUGIN') if e.get('uid')==old_uid])==1
  assert user_preset.read_bytes()==b'user preset sentinel; stable plugin UID'
  if not foreign:
   for rel,data in declared.items():assert (new_dir/rel).read_bytes()==data
  run(b,'uninstall.sh',env,target)
  assert not any(e.get('uid')==old_uid for e in ET.parse(settings).iter('PLUGIN'))
  assert len([e for e in ET.parse(settings).iter('PLUGIN') if e.get('uid')=='deadbeef'])==1
  assert user_preset.read_bytes()==b'user preset sentinel; stable plugin UID'
  if foreign:assert (old_dir/'foreign-file').read_bytes()==b'do not remove'
 print(('Foreign-id safety' if foreign else 'Old-to-new vendor upgrade')+': install, repeat, declared user settings, external user preset, native preset paths, stable UID and uninstall PASS')
print('Vendor migration PASS; no device involved')
