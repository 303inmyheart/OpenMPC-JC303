#!/usr/bin/env python3
"""Create an OpenMPC Installer compatible Live III development ZIP on the desktop.
Requires current successful native and actual ARM64 shared-object tests. Does not
install anything, restart MPC, publish a release, or include development files.
"""
import hashlib
import json
from pathlib import Path
import re
import shutil
import struct
import xml.etree.ElementTree as ET
import subprocess
import sys
import zipfile
import build

ROOT=build.ROOT
VERSION='0.2.2'
arm=ROOT/'build/aarch64'/build.CFG['so']
tested=json.loads((arm.parent/'tested.json').read_text())
if hashlib.sha256(arm.read_bytes()).hexdigest()!=tested['sha256']:
    raise SystemExit('ARM64 binary changed since the last passing test. Run tools/test_arm64.py.')
if 'JC303 behavioural tests PASSED' not in (ROOT/'build/test.log').read_text():
    raise SystemExit('Native sanitizer tests are missing or did not pass.')

def readelf(*args):
    return subprocess.check_output(['readelf',*args,str(arm)],text=True)
if 'AArch64' not in readelf('-h'):
    raise SystemExit('Wrong plugin architecture')
versions=[tuple(map(int,x.split('.'))) for x in re.findall(r'GLIBC_(\d+\.\d+)',readelf('-V'))]
if max(versions)>(2,39):
    raise SystemExit('Plugin requires a newer glibc than MPC Live III')
symbols=readelf('--dyn-syms','-W')
exports=[line.split()[-1] for line in symbols.splitlines()
         if ' GLOBAL ' in line and ' UND ' not in line]
if exports!=['VSTPluginMain']:
    raise SystemExit('Unexpected public symbols: '+str(exports))

stage=ROOT/'build/install'
folder=f"{build.CFG['vendor']} - VST - {build.CFG['name']}"
destination=stage/'VST2 Plugins'/folder
destination.mkdir(parents=True,exist_ok=True)
skin=ROOT/'build/skin'/folder
binary=destination/arm.name
shutil.copy2(arm,binary)
strip=shutil.which('aarch64-linux-gnu-strip') or str(build.LOCAL/'bin/aarch64-linux-gnu-strip')
build.run([strip,'--strip-unneeded',binary])

notices=destination/'LICENSES.txt'
notices.write_text('\n\n'.join([
    'JC303 MPC development port — GPL-3.0-only\nBased on midilab/jc303, commit 25d0ed1dab6d0edffc0e28c7d68c170a8af71aee.\nSource: https://github.com/303inmyheart/OpenMPC-JC303',
    (ROOT/'LICENSE').read_text(),
    'Open303 engine — MIT\n'+(ROOT/'src/dsp/open303/LICENSE').read_text(),
    'Aciduino sequencer and harmonizer — MIT\n'+(ROOT/'src/sequencer/LICENSE').read_text(),
    'MPC wrapper — MIT\n'+(build.MPC/'LICENSE').read_text(),
    'RTNeural — BSD-3-Clause\n'+(ROOT/'vendor/RTNeural/LICENSE').read_text(),
    'xsimd — BSD-3-Clause\n'+(ROOT/'vendor/xsimd/LICENSE').read_text(),
    'nlohmann JSON — MIT (notice in bundled header)\n'+(ROOT/'vendor/RTNeural/modules/json/json.hpp').read_text().split('#ifndef')[0],
    'ChowDSP RNN integration — GPL-3.0\n'+(ROOT/'src/drive/CHOWDSP-LICENSE.md').read_text(),
    'math_approx — BSD-3-Clause\n'+(ROOT/'vendor/math_approx/LICENSE').read_text(),
]))

# Use the official release packager rather than a folder-copy ZIP: the desktop
# installer validates scripts, manifests and checksums before uploading anything.
# Identify this fork as the modified source; preserve upstream attribution above.
release_dir=ROOT/'build/releases'
build.run([sys.executable,build.MPC/'tools/release.py',
           '--so',binary,'--skin',skin,'--entry',ROOT/'build/pluginlist-entry.xml',
           '--version',VERSION,'--id','jc303-mpc','--license','GPL-3.0-only',
           '--repo','303inmyheart/OpenMPC-JC303',
           '--extra',str(notices)+':LICENSES.txt',
           '--about','JC303 Acid / 303 hardware skin, optimized GuitarML drive, sequencer and 12 MPC browser programs. Init starts dry.',
           '--requires','MPC Live III, MPC OS 3.10.1 or newer',
           '-o',release_dir])
release=release_dir/f'JC303-MPC-{VERSION}-mpc-aarch64.zip'
build.run([sys.executable,build.MPC/'tools/catalog_check.py',release,'--require-portable'])
with zipfile.ZipFile(release) as archive:
    assert archive.testzip() is None
    top=next(iter({name.split('/')[0] for name in archive.namelist()}))
    required=('install.sh','uninstall.sh','plugin_list.awk','INSTALL.md',
              'SHA256SUMS','mpc-plugin.json')
    assert all(top+'/'+name in archive.namelist() for name in required)
    browser=top+'/portable/'+folder+'/Plugin Skins/browser_images/soundsmode.png'
    png=archive.read(browser)
    assert png[:8]==b'\x89PNG\r\n\x1a\n' and struct.unpack('>II',png[16:24])==(270,110)
    init=ET.fromstring(archive.read(top+'/portable/'+folder+'/Presets/0000-Init.xpl'))
    assert init.findtext('preset')=='Init' and init.find('PLUGIN').attrib['uid']=='4a634d33'
    assert not any(name.endswith('0000-Default.xpl') for name in archive.namelist())
    manifest=json.loads(archive.read(top+'/mpc-plugin.json'))
    assert manifest['arch']=='aarch64' and manifest['id']=='jc303-mpc'
    assert sum(name.endswith('.so') for name in archive.namelist())==1
    assert not any(name.endswith(('.o','.cpp','.c','.py','.log','.ttf')) for name in archive.namelist())
    for line in archive.read(top+'/SHA256SUMS').decode().splitlines():
        digest,name=line.split('  ',1)
        assert hashlib.sha256(archive.read(top+'/'+name)).hexdigest()==digest

desktop=Path.home()/'Schreibtisch'
desktop.mkdir(exist_ok=True)
output=desktop/f'JC303-MPC-Live-III-{VERSION}-OpenMPC.zip'
if output.exists():
    raise SystemExit('Refusing to overwrite '+str(output))
shutil.copy2(release,output)
print(output)
print(f'{output.stat().st_size:,} bytes; OpenMPC release structure and checksums OK')
print('SHA256',hashlib.sha256(output.read_bytes()).hexdigest())
