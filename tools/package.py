#!/usr/bin/env python3
"""Create an OpenMPC Installer compatible Live III development ZIP under build/releases.
Requires current successful native and actual ARM64 shared-object tests. Does not
install anything, restart MPC, publish a release, or include development files.
"""
import hashlib
import argparse
import os
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
CONFIG=json.loads((ROOT/'release.json').read_text())
VERSION=CONFIG['version']
ap=argparse.ArgumentParser(description=__doc__)
ap.add_argument('--desktop-name',help='Distinct installation ZIP name for a revised unpublished candidate')
ap.add_argument('--no-desktop',action='store_true',help='Keep artifacts under build/releases only (default)')
ap.add_argument('--desktop',action='store_true',help='Explicitly copy installer to Desktop')
ap.add_argument('--bench',type=Path,help='official device bench JSON/text for this binary')
a=ap.parse_args()
assert build.CFG['version']==int(VERSION.split('.')[0])*10000+int(VERSION.split('.')[1])*100+int(VERSION.split('.')[2])
RELEASE_TOOLS=Path(os.environ.get('MPC_RELEASE_TOOLS',ROOT.parent/'mpc-vst-plugins-release-tools')).resolve()
if not (RELEASE_TOOLS/'tools/release.py').exists():raise SystemExit('Missing release tools; see BUILDING.md.')
ref=subprocess.check_output(['git','-C',str(RELEASE_TOOLS),'rev-parse','HEAD'],text=True).strip()
if ref!=CONFIG['tools_ref']:raise SystemExit('Release tools must be pinned to '+CONFIG['tools_ref'])
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
    'JC303 MPC development port — combined GPLv3 / AGPLv3\nBased on midilab/jc303 v0.14.0, commit b0ff26011f80e45c7796cad7b29fbc7f347b9337; legacy engine 25d0ed1dab6d0edffc0e28c7d68c170a8af71aee.\nSource: https://github.com/303inmyheart/OpenMPC-JC303',
    (ROOT/'LICENSE').read_text(),
    'New DSP file-specific AGPL-3.0-or-later notices apply; see corresponding source.\n'+(ROOT/'src/dsp/v014/open303/LICENSE-AGPL-3.0').read_text(),
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
bench_args=[]
if a.bench:
    bench=json.loads(a.bench.read_text().strip().splitlines()[-1])
    if bench.get('binary_sha256')!=hashlib.sha256(arm.read_bytes()).hexdigest():
        raise SystemExit('CPU benchmark does not match this ARM64 binary')
    if bench['verdict']=='FAIL':raise SystemExit('CPU benchmark FAIL: optimize before release')
    bench_args=['--bench',a.bench]

build.run([sys.executable,RELEASE_TOOLS/'tools/release.py',
           '--so',binary,'--skin',skin,'--entry',ROOT/'build/pluginlist-entry.xml',
           '--version',VERSION,'--id',CONFIG['id'],'--license',CONFIG['license'],
           '--repo',CONFIG['repo'],
           '--extra',str(notices)+':LICENSES.txt',
           '--about',CONFIG['about'],
           '--requires',CONFIG['requires'],*bench_args,
           '-o',release_dir])
release=release_dir/f'JC303-MPC-{VERSION}-mpc-aarch64.zip'
build.run([sys.executable,RELEASE_TOOLS/'tools/catalog_check.py',release,'--catalog','--require-portable',
           '--expect-id',CONFIG['id'],'--expect-repo',CONFIG['repo']])
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

print('Catalog asset:',release)
if a.no_desktop or not a.desktop:raise SystemExit(0)
desktop=Path.home()/'Schreibtisch'
desktop.mkdir(exist_ok=True)
filename=a.desktop_name or f'JC303-MPC-Gen2-{VERSION}-OpenMPC.zip'
if not re.fullmatch(r'[A-Za-z0-9._-]+\.zip',filename):raise SystemExit('Invalid desktop ZIP filename')
output=desktop/filename
if output.exists():
    raise SystemExit('Refusing to overwrite '+str(output))
shutil.copy2(release,output)
print(output)
print(f'{output.stat().st_size:,} bytes; OpenMPC release structure and checksums OK')
print('SHA256',hashlib.sha256(output.read_bytes()).hexdigest())
