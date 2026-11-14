#!/usr/bin/env python3
"""Run the actual ARM64 .so in QEMU: load/link/ABI and behavioural checks, NOT a CPU benchmark.
Requires aarch64 build and qemu-aarch64 (on PATH or .tools/qemu/usr/bin).
"""
import shutil
import os
import hashlib
import json
import build
import framework_test

dest=build.ROOT/'build/aarch64'
common=build.flags('aarch64',False)
cc=build.compiler('aarch64','c')
cxx=build.compiler('aarch64','c++')
build.run([cc,'-std=gnu11',*common,'-c','tests/load_plugin.c','-o',dest/'load_plugin.o'])
qemu=shutil.which('qemu-aarch64') or str(build.ROOT/'.tools/qemu/usr/bin/qemu-aarch64')
# Installed Ubuntu cross packages and the private unpacked toolchain use
# different sysroot locations. QEMU_LD_PREFIX can explicitly select another.
sysroots=[build.Path(os.environ['QEMU_LD_PREFIX'])] if os.environ.get('QEMU_LD_PREFIX') else [
    build.LOCAL/'aarch64-linux-gnu',build.Path('/usr/aarch64-linux-gnu')]
sysroot=next((p for p in sysroots if p.exists()),None)
if sysroot is None:raise SystemExit('ARM64 runtime missing; set QEMU_LD_PREFIX to its sysroot.')
for source,name in [(framework_test.source(build,dest),'host_test'),(build.ROOT/'tests/voice_test.cpp','voice_test')]:
    cpp=source.suffix=='.cpp'
    obj=dest/(name+'.o')
    build.run([cxx if cpp else cc,'-std=c++20' if cpp else '-std=gnu11',*common,
               '-Isrc','-Dmain=plugin_test_main','-c',source,'-o',obj])
    # C++ test entry needs C linkage for the loader.
    if cpp:
        # A tiny C++ bridge keeps the source identical to the native tests.
        bridge=dest/'bridge.cpp'
        bridge.write_text('int plugin_test_main();\nextern "C" int run_cpp_tests(){return plugin_test_main();}\n')
        build.run([cxx,*common,'-c',bridge,'-o',dest/'bridge.o'])
        build.run([cc,*common,'-Dplugin_test_main=run_cpp_tests','-c','tests/load_plugin.c','-o',dest/'cpp_loader.o'])
        extras=[dest/'cpp_loader.o',dest/'bridge.o']+[p for p in dest.glob('*.o')
                if p.stem in {build.object_name(s) for s in build.CFG['build']['sources'] if s!='src/engine.cpp'}]
    else:
        extras=[dest/'load_plugin.o']
    binary=dest/name
    build.run([cxx,*common,obj,*extras,'-static-libstdc++','-static-libgcc','-ldl','-lpthread','-lm','-o',binary])
    build.run([qemu,'-L',sysroot,binary,dest/build.CFG['so']])

(dest/'tested.json').write_text(json.dumps({
    'sha256': hashlib.sha256((dest/build.CFG['so']).read_bytes()).hexdigest(),
    'tests': ['framework host', 'reference voice', 'state and MIDI', '54 filter/LFO combinations',
              'stepped Q-Link host/touch synchronisation with host echoes',
              'six-waveform rotary sweeps and one-second slide migration',
              'legacy sequencer exact audio/migration, 64-step grid/pages, generator length, swing/fractional gates, CPU timing oracle, scaled host clock, step articulation DSP, JC8 state and 12-key/19-scale interval oracle',
              '37 GuitarML models, independent LSTM oracle, rapid switching and bypass',
              'four-slot modulation matrix, all source/target routes, bipolar/Overdrive routing and old-state migration',
              'Host Transport, Note Gate/Hold and latched Note Trigger sample/timing/state checks',
              '12 MPC browser programs, dry Init, edited state restore and wet VST audio'],
    'environment': 'QEMU ARM64, offline only; no hardware benchmark',
},indent=2)+'\n')
