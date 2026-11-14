#!/usr/bin/env python3
"""Build locally without Docker. Usage: python3 tools/build.py [native|aarch64|skin|all].
MPC_VST selects the neighbouring toolchain checkout. AARCH64_CXX/AARCH64_CC may
select installed cross compilers; the project-local unpacked Ubuntu toolchain
is used otherwise. Native outputs are development artifacts, never device files.
"""
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
import embed_models
import sequence_spec

ROOT = Path(__file__).resolve().parents[1]
MPC = Path(os.environ.get('MPC_VST', ROOT.parent / 'mpc-vst-plugins')).resolve()
LOCAL = ROOT / '.tools/toolchain/usr'
CFG = json.loads((ROOT / 'vst.json').read_text())

def run(args, **kwargs):
    env = os.environ.copy()
    local_lib = str(LOCAL/'lib/x86_64-linux-gnu')
    env['LD_LIBRARY_PATH'] = local_lib + ':' + env.get('LD_LIBRARY_PATH', '')
    env['PYTHONPATH'] = str(LOCAL/'lib/python3/dist-packages') + ':' + env.get('PYTHONPATH', '')
    if CFG.get('title_font'):
        env['SHADOW_TITLE_FONT'] = str(ROOT/CFG['title_font'])
    subprocess.run([str(x) for x in args], cwd=ROOT, check=True, env=env, **kwargs)

def compiler(target, language):
    if target == 'native':
        return os.environ.get('CXX' if language == 'c++' else 'CC', 'g++' if language == 'c++' else 'gcc')
    override = os.environ.get('AARCH64_CXX' if language == 'c++' else 'AARCH64_CC')
    if override:
        return override
    tool = 'aarch64-linux-gnu-g++' if language == 'c++' else 'aarch64-linux-gnu-gcc'
    for candidate in (tool,tool+'-13'):
        if shutil.which(candidate):
            return candidate
    local = LOCAL / 'bin' / (tool + '-13')
    if not local.exists():
        raise SystemExit('Missing ARM64 compiler: see BUILDING.md or set AARCH64_CC/AARCH64_CXX.')
    return str(local)

def prepare():
    sequence_spec.generate()
    if not (MPC/'wrapper/audio_format.h').exists():
        raise SystemExit('Apply patches/mpc-float32.patch to MPC_VST first; see BUILDING.md.')
    run([sys.executable, MPC/'tools/gen_vst.py', ROOT/'vst.json', '--params-h'])

def flags(target, sanitize):
    result = ['-fPIC', '-fvisibility=hidden', '-Ibuild', '-I'+str(MPC/'wrapper'),
              '-Ivendor/math_approx/include', '-Ivendor/RTNeural', '-Ivendor/xsimd/include',
              '-Isrc/drive', '-Wall', '-Wextra', '-Wno-unused-parameter']
    result += ['-O1', '-g', '-fsanitize=address,undefined', '-fno-omit-frame-pointer'] if sanitize else ['-O2']
    if target == 'aarch64' and str(LOCAL) in compiler(target, 'c++'):
        result += ['--sysroot='+str(LOCAL.parent), '-B'+str(LOCAL/'bin')+'/']
    return result

def object_name(source):
    return ("v014_" if "/v014/" in str(source) else "")+Path(source).stem

def build(target, sanitize=False):
    prepare()
    embed_models.generate(ROOT)
    dest = ROOT / 'build' / (target + ('-san' if sanitize else ''))
    dest.mkdir(parents=True, exist_ok=True)
    common = flags(target, sanitize)
    objects = []
    for source in CFG['build']['sources'] + [str(MPC/'wrapper/vst2_wrap.c')]:
        cpp = source.endswith('.cpp')
        obj = dest / (object_name(source)+'.o')
        run([compiler(target, 'c++' if cpp else 'c'), '-std=c++20' if cpp else '-std=gnu11',
             *common, *(['-O3'] + (['-mtune=cortex-a76'] if target=='aarch64' else []) if source.endswith('RNNAccelerated.cpp') and not sanitize else []), '-c', source, '-o', obj])
        objects.append(obj)
    output = dest / CFG['so']
    run([compiler(target, 'c++'), *common, '-shared', *objects, '-lpthread', '-lm', '-ldl',
         '-Wl,--no-undefined', '-Wl,--version-script='+str(ROOT/'src/exports.map'),
         *(['-static-libstdc++', '-static-libgcc'] if target == 'aarch64' else []), '-o', output])
    print('Built:', output, flush=True)
    return objects

def skin():
    (ROOT/'build').mkdir(exist_ok=True)
    run(['gcc','-O2','-I'+str(MPC/'tools/vendor/force-shadow/tools'),
         MPC/'tools/shadow_art.c','-lm','-o',ROOT/'build/shadow_art'])
    generated=ROOT/'build/skin'/f"{CFG['vendor']} - VST - {CFG['name']}"
    if generated.exists(): shutil.rmtree(generated)
    run([sys.executable, ROOT/'tools/browser_tile.py'])
    run([sys.executable, MPC/'tools/gen_vst.py', ROOT/'vst.json'])
    run([sys.executable, ROOT/'tools/finish_skin.py'])
    run([sys.executable, ROOT/'tools/sequencer_skin.py'])
    run([sys.executable, ROOT/'tools/model_overlay.py'])
    run([sys.executable, ROOT/'tools/browser_preset.py'])
    run([sys.executable, ROOT/'tools/group_mod_pages.py'])
    folder=next((ROOT/'build/skin').glob('*/Plugin Skins/TUI.json')).parent
    run([sys.executable, MPC/'tools/studio.py','preview',folder,'-o', ROOT/'build/preview_%d.png'])

if __name__ == '__main__':
    target = sys.argv[1] if len(sys.argv) > 1 else 'all'
    if target not in ('native','aarch64','skin','all'):
        raise SystemExit('Choose native, aarch64, skin, or all')
    if target in ('native','all'): build('native')
    if target in ('aarch64','all'): build('aarch64')
    if target in ('skin','all'): skin()
