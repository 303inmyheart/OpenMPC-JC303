#!/usr/bin/env python3
"""Render the built native skin using values queried from the actual VST2 .so.
Offline documentation previews, not device screenshots. Uses DejaVu in place of
MPC's host fonts. No debug/focus rectangles are added to the public screenshots.
"""
import ctypes as C
import json
import os
from pathlib import Path
import sys
import time
from PIL import Image,ImageDraw,ImageFont
ROOT=Path(__file__).resolve().parents[1]
MPC=Path(os.environ.get('MPC_VST',ROOT.parent/'mpc-vst-plugins'))
sys.path.insert(0,str(MPC/'tools'))
import studio
cfg=json.loads((ROOT/'vst.json').read_text())
params=json.loads((ROOT/'params.json').read_text())['params']
ids={p['key']:i for i,p in enumerate(params)}
skin=ROOT/'build/skin'/f"{cfg['vendor']} - VST - {cfg['name']}"/'Plugin Skins'
tui=json.loads((skin/'TUI.json').read_text())['pageData']
defs={d['key']:d['value'] for d in tui['componentDefinitions']['localComponentDefinitions']}
out=ROOT/'docs/screenshots';out.mkdir(parents=True,exist_ok=True)
class Effect(C.Structure):
    _fields_=[('magic',C.c_int32),('dispatch',C.c_void_p),('process',C.c_void_p),
              ('set',C.c_void_p),('get',C.c_void_p)]
Call=C.CFUNCTYPE(C.c_ssize_t,C.c_void_p,C.c_int32,C.c_int32,C.c_ssize_t,C.c_void_p,C.c_float)
callback=Call(lambda fx,op,i,v,p,o:2400 if op==1 else 0)
lib=C.CDLL(str(ROOT/'build/native'/cfg['so']))
lib.VSTPluginMain.restype=C.POINTER(Effect)
e=lib.VSTPluginMain(callback)
assert e and e.contents.magic==0x56737450
call=Call(e.contents.dispatch)
get=C.CFUNCTYPE(C.c_float,C.c_void_p,C.c_int32)(e.contents.get)
setparam=C.CFUNCTYPE(None,C.c_void_p,C.c_int32,C.c_float)(e.contents.set)
# processReplacing offset in the VST2 AEffect ABI on the native 64-bit host.
# Rendering isn't required for captions; readouts use the same dispatcher as MPC.
call(e,0,0,0,None,0)
def display(i):
    buf=C.create_string_buffer(128);call(e,7,i,0,buf,0)
    return buf.value.decode('utf-8',errors='replace')
def xywh(bounds):return tuple(int(float(x)) for x in bounds['bounds'].split())
try:
    for n,tab in enumerate(tui['tabs']):
        role=tab['componentName'].split('|')[0]
        call(e,2,0,10 if role in ('SEQUENCE','GENERATE','ARTIC','SEQTOOLS','SEQLIVE','SEQKEY') else 0,None,0)
        if role.startswith('MATRIX'):setparam(e,ids['engine014'],1);setparam(e,ids['switchModState'],1)
        if role in ('MODS','LFO','SETUP'):setparam(e,ids['engine014'],1);setparam(e,ids['switchModState'],1)
        if role=='LEGACY':setparam(e,ids['engine014'],0);setparam(e,ids['switchModState'],1)
        if role=='LFO':setparam(e,ids['lfoDepth'],.4)
        if role=='DRIVE':
            # Keep the documentation shot dry: all model choices are available without loading a network.
            setparam(e,ids['driveModel'],16/36)
        if role in ('ENGINE','FILTER'):
            setparam(e,ids['engine014'],1);setparam(e,ids['switchModState'],1)
            setparam(e,ids['filterModel014'],1/7)
        if role in ('SEQUENCE','GENERATE','ARTIC','SEQTOOLS','SEQLIVE','SEQKEY'):
            setparam(e,ids['seqTiming'],1);setparam(e,ids['seqLength64'],1)
        if role in ('SEQUENCE','SEQKEY'):
            setparam(e,ids['seqKeyRoot'],9/11);setparam(e,ids['seqKeyScale'],2/19)
        state={i:round(get(e,i)*(len(q['options'])-1)) for i,q in enumerate(params) if q.get('options')}
        im=Image.new('RGBA',(1280,628));draw=ImageDraw.Draw(im)
        for c in studio.tab_components(tab,defs):
            if not studio.shown(c,state):continue
            cd=c['componentData'];x,y,ww,hh=xywh(c['bounds'])
            if cd['type']=='Image':
                pic=Image.open(skin/cd['data']['image']).convert('RGBA');im.alpha_composite(pic,(x,y));continue
            mapping={p['key']:int(p['value'].split()[-1]) for p in c['handle remapping']['map']}
            for child in defs[cd['type']]['componentsData']:
                sd=child['componentData'];d=sd['data'];sx,sy,w,h=xywh(child['bounds'])
                i=mapping.get(d.get('handleName','Data'),mapping.get('Data',-1))
                value=get(e,i) if i>=0 else 0
                if sd['type']=='Knob':
                    pic=Image.open(skin/d['filmStrip']).convert('RGBA');size=pic.width
                    frames=pic.height//size;k=max(0,min(frames-1,round(value*(frames-1))))
                    im.alpha_composite(pic.crop((0,k*size,size,(k+1)*size)),(x+sx,y+sy))
                elif sd['type']=='Button':
                    count=d['numButtonsInGroup'];bid=d['buttonId']
                    if 43<=i<=58:value=1 if i-43==round(get(e,25)*15) else 0
                    on=round(value*max(count-1,1))==bid
                    pic=Image.open(skin/d['onImage' if on else 'offImage']).convert('RGBA')
                    im.alpha_composite(pic,(x+sx,y+sy))
                elif sd['type']=='Label':
                    caption=cd['name'] if d.get('type')=='Name' else display(i)
                    style=d['textStyle'];color=style['colour'];ink='#'+color[-6:]
                    px=round(style['font']['height'])
                    f=ImageFont.truetype(str(ROOT/'art/DejaVuSans.ttf'),px)
                    draw.text((x+sx+w/2,y+sy+h/2),caption,font=f,fill=ink,anchor='mm')
        target=out/(tab['tabName'].lower()+'.png');im.convert('RGB').save(target)
        print(target.relative_to(ROOT))
finally:call(e,1,0,0,None,0)
