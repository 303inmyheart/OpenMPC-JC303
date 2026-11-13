#!/usr/bin/env python3
"""Build and test the native voice + actual VST2 wrapper under ASan/UBSan."""
import os
from pathlib import Path
import build

os.environ['ASAN_OPTIONS']='detect_leaks=1:halt_on_error=1'
os.environ['UBSAN_OPTIONS']='halt_on_error=1:print_stacktrace=1'
objects=build.build('native',sanitize=True)
dest=build.ROOT/'build/native-san'
common=build.flags('native',True)
for source,name in [(build.MPC/'tools/host_test.c','host_test'),(build.ROOT/'tests/voice_test.cpp','voice_test')]:
    cpp=source.suffix=='.cpp'
    obj=dest/(name+'.o')
    build.run([build.compiler('native','c++' if cpp else 'c'),'-std=c++20' if cpp else '-std=gnu11',
               *common,'-Isrc','-c',source,'-o',obj])
    binary=dest/name
    build.run(['g++',*common,obj,*objects,'-lpthread','-ldl','-lm','-o',binary])
    build.run([binary])
