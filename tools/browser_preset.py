#!/usr/bin/env python3
"""Export a real Init chunk so the Sounds-browser tile has a loadable preset."""
import ctypes as C
import json
import os
from pathlib import Path
import sys
import xml.etree.ElementTree as ET
import factory_presets
ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(Path(os.environ.get('MPC_VST',ROOT.parent/'mpc-vst-plugins'))/'tools'))
import xpl
cfg=json.loads((ROOT/'vst.json').read_text())
folder=f"{cfg['vendor']} - VST - {cfg['name']}"
skin=ROOT/'build/skin'/folder
uid=int.from_bytes(cfg['uid'].encode(),'big')

def native_xpl(chunk,name):
    tree=ET.fromstring(xpl.xpl(cfg['name'],cfg['vendor'],
        f'%payload-path%/{folder}/{cfg["so"]}',uid,cfg['version'],chunk,preset=name))
    tree.find('state').text=xpl.b64enc(xpl.fxb_chunk_set(uid,cfg['version'],chunk,
        num_programs=cfg['defines']['ENGINE_PROGRAM_COUNT']))
    return ET.tostring(tree,encoding='unicode',xml_declaration=True)

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
    assert chunk.startswith(b'JC303-MPC-10\n') and b'preset 0\n' in chunk
    assert get(effect,8)==0 and all(get(effect,i)==0 for i in range(603,610)), 'Init must disable every MODS switch'
    text=native_xpl(chunk,'Init')
    # Verify the actual exported JUCE bank bytes, then restore them over a wet preset.
    encoded=ET.fromstring(text).findtext('state')
    bank=xpl.b64dec(encoded)
    assert bank[:4]==b'CcnK' and bank[8:12]==b'FBCh'
    assert int.from_bytes(bank[16:20],'big')==uid
    assert int.from_bytes(bank[24:28],'big')==30
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
    manifest=[]
    params=json.loads((ROOT/'params.json').read_text())['params']
    originals=factory_presets.records(ROOT)
    for index in range(1,30):
        # Start each exported sound from a clean Init (including its sequence).
        call(effect,2,0,0,None,0)
        call(effect,2,0,index,None,0)
        name=C.create_string_buffer(64)
        call(effect,29,index,0,name,0)
        name=name.value.decode()
        assert name
        if index>=12:
            original=originals[index-12]
            assert name==original['name']
            for param,value in original['values']:
                p=params[param]
                low,high=(0,len(p['options'])-1) if 'options' in p else (p['min'],p['max'])
                expected=(value-low)/(high-low) if high!=low else 0
                assert abs(get(effect,param)-expected)<2e-6,(name,p['key'],get(effect,param),expected)
        size=call(effect,23,0,0,C.byref(ptr),0)
        state=C.string_at(ptr,size)
        assert f'preset {index}\n'.encode() in state
        text=native_xpl(state,name)
        # Restore the actual exported bank over Init; retain the selected name.
        tree=ET.fromstring(text)
        bank=xpl.b64dec(tree.findtext('state'))
        assert bank[160:]==state
        call(effect,2,0,0,None,0)
        buffer=C.create_string_buffer(bank[160:])
        call(effect,24,0,len(state),buffer,0)
        assert call(effect,3,0,0,None,0)==index
        size2=call(effect,23,0,0,C.byref(ptr),0)
        assert C.string_at(ptr,size2)==state,(name,'state failed roundtrip')
        prefix='Factory - ' if index>=12 else 'MPC - '
        filename=f'{index:04d}-{prefix}{name}.xpl'
        (presets/filename).write_text(text)
        manifest.append(dict(index=index,name=name,file=filename,source=originals[index-12]['source'] if index>=12 else 'MPC'))
    (skin/'Factory-Licenses.txt').write_text((ROOT/'factory/NOTICE.md').read_text()+'\n'+(ROOT/'LICENSE').read_text())
    (ROOT/'build/native-preset-manifest.json').write_text(json.dumps(manifest,indent=2)+'\n')
    print('Sounds browser: real Init .xpl exported and wet-to-dry restore verified.')
    print('Native MPC bank: 12 existing sounds plus 18 original factory sounds; all mapped values and full state roundtrips PASS.')
finally:
    call(effect,1,0,0,None,0)
