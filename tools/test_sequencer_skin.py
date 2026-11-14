#!/usr/bin/env python3
"""Regression: native SDK toggle wiring and built-skin bindings into the real VST.
This interprets documented button semantics, not MPC's actual touch event loop.
"""
import ctypes as C
import json,time
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
params=json.loads((ROOT/'params.json').read_text())['params'];ids={p['key']:i for i,p in enumerate(params)}
cfg=json.loads((ROOT/'vst.json').read_text())
skin=ROOT/'build/skin'/f"{cfg['vendor']} - VST - {cfg['name']}"/'Plugin Skins'
pd=json.loads((skin/'TUI.json').read_text())['pageData']
defs={d['key']:d['value'] for d in pd['componentDefinitions']['localComponentDefinitions']}
pages={t['tabName']:defs[t['componentName']]['componentsData'] for t in pd['tabs']}
# Function keys/subtabs and the Q-Link map must point at the same new pages.
tabs={t['tabName']:t for t in pd['tabs']}
banks=json.loads((skin/'Q-Links.json').read_text())['Screen Mode Q-Links']['map']
def bank(name):
    t=tabs[name];return next(b['Q-Links'] for b in banks if (b['Tab'],b['SubTab'])==(t['fnKeyIndex']+1,t['fnKeySubIndex']+1))
# Only rotary controls are assigned, with the explicit Drive-model exception.
for name,components in pages.items():
    expected={int(m['value'].split()[1]) for c in components if c['componentData']['type'].startswith('shKnob') for m in c['handle remapping']['map'] if m['key']=='Data'}
    if name=='DRIVE':expected.add(ids['driveModel'])
    assigned=[p for p in bank(name).values() if p>=0]
    assert set(assigned)==expected and len(assigned)==len(expected),(name,'only visible knobs/model may be mapped',assigned,expected)
    t=tabs[name];b=next(b for b in banks if (b['Tab'],b['SubTab'])==(t['fnKeyIndex']+1,t['fnKeySubIndex']+1))
    assert b['Bank Direction']=='RowTopDown'
    for row in range(4):
        members={b['Q-Links'][f'Q-Link {13+col-4*row}'] for col in range(4)}-{-1}
        if not members:continue
        x,y,w,h=map(int,t['qlinkBoundsData'][row].split())
        for c in components:
            if c['componentData']['type']=='Image':continue
            matches=[int(m['value'].split()[1]) for m in c.get('handle remapping',{}).get('map',[]) if m['key']=='Data']
            if not any(i in members for i in matches):continue
            xx,yy,ww,hh=rect(c) if 'rect' in globals() else tuple(map(int,c['bounds']['bounds'].split()))
            assert x<=xx and y<=yy and xx+ww<=x+w and yy+hh<=y+h,(name,row,'focus cuts a native label/value')
assert ids['seqTriggerStop'] not in bank('SEQTOOLS').values()
for slot in range(1,5):
    name=f'MATRIX{slot}';assert tabs[name]['fnKeyIndex']==tabs['LFO']['fnKeyIndex']
    assert 0<=tabs[name]['fnKeySubIndex']<8
    assert ids[f'modSlot{slot}Enabled'] not in bank(name).values()
    choices=[c for c in pages[name] if c['componentData']['type'].startswith(f'shSeg_matrixDest{slot}_')]
    assert len(choices)==14
overlay=defs['jc303 model overlay']
assert overlay['hideQLinkBounds'] and overlay['ignoreMousePresses']
assert overlay['componentsData'][0]['componentData']['type']=='KnobOverlay'
assert any(a['handler']=='Focus Lost' for a in overlay['componentsData'][0]['componentData']['data']['actions'])
assert not any(c['componentData']['type']=='Image' and c['bounds']['bounds']=='0 0 1280 800' for c in overlay['componentsData']),'full-screen child would swallow outside dismissal taps'
field=next(c for c in pages['DRIVE'] if c['componentData']['type'].startswith('shPopField_'))
assert all(a['handler']=='Show Overlay' and a['additionalData']=='jc303 model overlay' for a in defs[field['componentData']['type']]['actions'])
assert any(m['key']=='Data' and m['value']==f'Parameter {ids["driveModel"]}' for m in field['handle remapping']['map'])
print('Native Drive overlay still owns focus/dismissal and covers page Q-Link frames; Drive model binding retained.')
print('Six navigation groups; ONLY visible knobs plus Drive-model exception assigned; every bank frame encloses native controls/labels/values; matrix routing stays native touch.')
def binding(c):
    return next(int(m['value'].split()[1]) for m in c['handle remapping']['map'] if m['key']=='Data')
def rect(c):return tuple(map(int,c['bounds']['bounds'].split()))
# The existing MODS switch is the device-tested persistent-toggle reference.
reference=next(c for c in pages['MODS'] if c['componentData']['type'].startswith('shToggle'))
referenceDef=defs[reference['componentData']['type']]
def actions_of(d):return [(a['onAction'],a['handler'],a['handleName']) for a in d['actions']]
assert actions_of(defs['seqFlag'])==actions_of(referenceDef),'grid switch differs from the working MODS switch'
checked=0
for name in ('SEQUENCE','ARTIC'):
    for c in pages[name]:
        kind=c['componentData']['type']
        if kind not in ('seqNoteTap','seqOctTap','seqFlag'):continue
        d=defs[kind];actions=d['actions']
        expected='Q-Link' if kind=='seqFlag' else 'Toggle Switch'
        assert any(a['onAction']=='Mouse Down' and a['handler']==expected and a['handleName']=='Data' for a in actions),(name,kind,'wrong persistent/trigger recipe')
        if kind=='seqFlag':
            assert c['bounds']['acceptsHWFocus']=='Yes'
            assert not any(a['onAction']=='Mouse Down' and a['handler']=='Toggle Switch' for a in actions),'persistent flag would toggle twice'
        assert not d['hideQLinkBounds']
        buttons=[x for x in d['componentsData'] if x['componentData']['type']=='Button']
        assert len(buttons)==1
        button=buttons[0]['componentData']['data']
        assert button['buttonId']==1 and button['numButtonsInGroup']==1,(kind,'SDK toggle must target ON')
        assert not d['ignoreMousePresses']
        p=params[binding(c)];assert not p.get('live'),p['key']
        checked+=1
assert checked==432,checked # 192 notes + 64 octaves + 48 front flags + 128 articulation flags
class Effect(C.Structure):
    _fields_=[('magic',C.c_int32),('dispatch',C.c_void_p),('process',C.c_void_p),('set',C.c_void_p),('get',C.c_void_p)]
Call=C.CFUNCTYPE(C.c_ssize_t,C.c_void_p,C.c_int32,C.c_int32,C.c_ssize_t,C.c_void_p,C.c_float)
callback=Call(lambda fx,op,i,v,p,o:2400 if op==1 else 0)
lib=C.CDLL(str(ROOT/'build/native'/cfg['so']));lib.VSTPluginMain.restype=C.POINTER(Effect)
e=lib.VSTPluginMain(callback);assert e.contents.magic==0x56737450
call=Call(e.contents.dispatch)
get=C.CFUNCTYPE(C.c_float,C.c_void_p,C.c_int32)(e.contents.get)
setparam=C.CFUNCTYPE(None,C.c_void_p,C.c_int32,C.c_float)(e.contents.set)
def units(key,value):
    p=params[ids[key]];span=len(p['options'])-1 if p.get('options') else p['max']-p['min'];lo=0 if p.get('options') else p['min']
    setparam(e,ids[key],(value-lo)/span)
def value(key):
    p=params[ids[key]];n=get(e,ids[key]);return n*(len(p['options'])-1) if p.get('options') else p['min']+n*(p['max']-p['min'])
def tap(page,key):
    target=ids[key];c=next(c for c in pages[page] if c['componentData']['type'] in ('seqNoteTap','seqOctTap','seqFlag') and binding(c)==target)
    # The native SDK's single-button toggle resolves to 1 minus canonical state.
    # Resolve the handle from the actual generated skin, not a hard-coded engine call.
    setparam(e,binding(c),1-get(e,binding(c)))
def tap_at(page,x,y):
    hits=[]
    for c in pages[page]:
        if c['componentData']['type']!='seqFlag':continue
        cx,cy,w,h=rect(c)
        if cx<=x<cx+w and cy<=y<cy+h:hits.append(c)
    assert len(hits)==1,(page,x,y,'ambiguous/missing hit target')
    c=hits[0];key=params[binding(c)]['key'];tap(page,key);return key
call(e,0,0,0,None,0)
try:
    units('seqLength64',64);units('seqPage',3);units('seqSelected64',64);units('seqRest',1)
    tap('SEQUENCE','seqCell16_9')
    assert round(value('seqNote'))%12==9 and value('seqRest')==0 and round(value('seqSelected64'))==64
    assert get(e,ids['seqCell16_9'])==0,'momentary action did not spring back'
    tap('SEQUENCE','seqOctCell16_0');assert round(value('seqNote'))==81,'octave touch did not reach the fourth-page note'
    tap('SEQUENCE','seqGridSlide16');assert value('seqSlide')==1
    tap('SEQUENCE','seqGridSlide16');assert value('seqSlide')==0
    tap('ARTIC','seqGridAccent16');assert value('seqAccent')==1
    tap('ARTIC','seqGridMute16');assert value('seqMute')==1 and value('seqAccent')==0
    tap('ARTIC','seqGridHammer16');assert value('seqHammer')==1 and value('seqSlide')==0
    time.sleep(.27);tap('SEQUENCE','seqCell16_9');assert value('seqRest')==1,'second note tap did not remove note'
    # Follow the visible cell centers, not preselected parameter IDs. Every lane
    # on both pages must turn ON and OFF and resolve to its printed step/flag.
    units('seqPage',0)
    kinds=('Gate','Slide','Accent','Vibrato','Tie','Mute','Hammer','Reverse')
    for col in range(16):
        for row,kind in enumerate(kinds):
            key=f'seqGrid{kind}{col+1}';units(key,0)
            assert tap_at('ARTIC',144+67*col,298+32*row)==key
            assert round(value(key))==1,(key,'cannot turn on')
            tap_at('ARTIC',144+67*col,298+32*row)
            assert round(value(key))==0,(key,'cannot turn off')
        for kind,y in (('Slide',513),('Accent',543),('Vibrato',573)):
            key=f'seqGrid{kind}{col+1}';units(key,0)
            assert tap_at('SEQUENCE',144+67*col,y)==key
            assert round(value(key))==1,(key,'front lane cannot turn on')
            tap_at('SEQUENCE',144+67*col,y)
            assert round(value(key))==0,(key,'front lane cannot turn off')
    units('seqSelected64',1);units('seqNote',36);units('seqRest',0)
    tap_at('ARTIC',144,426) # TIE is a tied rest, so GATE turns off.
    assert value('seqTie')==1 and value('seqRest')==1
    tap_at('ARTIC',144,298) # Explicit GATE restores the stored note and clears TIE.
    assert value('seqRest')==0 and value('seqTie')==0 and round(value('seqNote'))==36
    units('seqKeyRoot',9);units('seqKeyScale',2)
    major=next(c for c in pages['SEQKEY'] if any(m['value']==f'Parameter {ids["seqKeyScale"]}' for m in c.get('handle remapping',{}).get('map',[])))
    # The original key/scale controls are native radio groups; highlight images
    # must remain behind every note's touch target.
    rows=[c for c in pages['SEQUENCE'] if c['componentData']['type']=='Image' and c['componentData']['data']['image']=='seq_scale_row.png']
    assert len(rows)==12
    firstInput=min(i for i,c in enumerate(pages['SEQUENCE']) if c['componentData']['type']=='seqNoteTap')
    assert all(pages['SEQUENCE'].index(c)<firstInput for c in rows),'scale artwork obscures a note input'
    for pitch in range(12):assert bool(round(value(f'seqScaleView{pitch}')))==(pitch in (0,2,4,5,7,9,11))
    lines=[c for c in pages['SEQUENCE'] if c['componentData']['type']=='Image' and c['componentData']['data']['image']=='seq_running.png']
    assert len(lines)==16
    for col,c in enumerate(lines):
        x,y,w,h=rect(c);assert (x,y,w,h)==(110+col*67,130,2,458)
        assert c['bounds']['additionalInvalidatingHandles']==[f'IndexedEnabling/1/2/Parameter {ids[f"seqPlayView{col+1}"]}']
    print(f'{checked} native grid controls, all 176 flag-cell hit targets and ON/OFF transitions:  SDK toggle actions/IDs verified; real VST note, octave, flag and page bindings PASS; 16 playhead lines verified. Actual MPC touch acceptance pending.')
finally:call(e,1,0,0,None,0)
