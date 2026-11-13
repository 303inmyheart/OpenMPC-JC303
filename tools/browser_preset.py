#!/usr/bin/env python3
"""Export a real Init chunk so the Sounds-browser tile has a loadable preset."""
import ctypes as C
import json
import os
from pathlib import Path
import sys
import xml.etree.ElementTree as ET
ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(Path(os.environ.get('MPC_VST',ROOT.parent/'mpc-vst-plugins'))/'tools'))
import xpl
cfg=json.loads((ROOT/'vst.json').read_text())
folder=f"{cfg['vendor']} - VST - {cfg['name']}"
skin=ROOT/'build/skin'/folder

class Effect(C.Structure):
    _fields_=[('magic',C.c_int32),('dispatch',C.c_void_p),('process',C.c_void_p),
              ('set',C.c_void_p),('get',C.c_void_p)]
Call=C.CFUNCTYPE(C.c_ssize_t,C.c_void_p,C.c_int32,C.c_int32,C.c_ssize_t,C.c_void_p,C.c_float)
callback=Call(lambda fx,op,i,v,p,o:2400 if op==1 else 0)
lib=C.CDLL(str(ROOT/'build/native'/cfg['so']))
lib.VSTPluginMain.restype=C.POINTER(Effect)
effect=lib.VSTPluginMain(callback)
assert effect and effect.contents.magic==0x56737450
call=Call(effect.contents.dispatch)
get=C.CFUNCTYPE(C.c_float,C.c_void_p,C.c_int32)(effect.contents.get)
call(effect,0,0,0,None,0)
try:
    ptr=C.c_void_p()
    size=call(effect,23,0,0,C.byref(ptr),0)
    assert size>0
    chunk=C.string_at(ptr,size)
    assert chunk.startswith(b'JC303-MPC-4\n') and b'preset 0\n' in chunk
    uid=int.from_bytes(cfg['uid'].encode(),'big')
    text=xpl.xpl(cfg['name'],cfg['vendor'],f'%payload-path%/{folder}/{cfg["so"]}',
                 uid,cfg['version'],chunk,preset='Init')
    # Verify the actual exported JUCE bank bytes, then restore them over a wet preset.
    encoded=ET.fromstring(text).findtext('state')
    bank=xpl.b64dec(encoded)
    assert bank[:4]==b'CcnK' and bank[8:12]==b'FBCh'
    assert int.from_bytes(bank[16:20],'big')==uid
    restored=bank[160:]
    assert restored==chunk and int.from_bytes(bank[156:160],'big')==len(chunk)
    call(effect,2,0,9,None,0)
    assert get(effect,59)==1
    buf=C.create_string_buffer(restored)
    call(effect,24,0,len(restored),buf,0)
    assert call(effect,3,0,0,None,0)==0 and get(effect,59)==0 and get(effect,22)==0
    presets=skin/'Presets';presets.mkdir(exist_ok=True)
    default=presets/'0000-Default.xpl'
    if default.exists():default.unlink()
    (presets/'0000-Init.xpl').write_text(text)
    print('Sounds browser: real Init .xpl exported and wet-to-dry restore verified.')
finally:
    call(effect,1,0,0,None,0)
