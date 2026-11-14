"""Append the v0.14 matrix API after the frozen 0.4.2 parameter table."""
from pathlib import Path
import json
ROOT=Path(__file__).resolve().parents[1]
SOURCES=['Off','LFO','Env','Accent','Key','Random','Cutoff','Env+']
DESTS=['Cutoff','Volume','Pitch','Pulse Width','Resonance','Overdrive','Filter FM','Env Mod','Decay','Accent','Waveform','LFO Rate','LFO Depth','Filter Morph']
def generate():
    path=ROOT/'params.json';data=json.loads(path.read_text());params=data['params']
    assert len(params)==588,'sequence API changed: review append-only matrix offsets'
    first=len(params);defs=[]
    def add(key,name,lo=0,hi=1,default=0,options=None,**kw):
        p=dict(key=key,name=name,min=lo,max=hi,default=default,**kw)
        if options:p['options']=options
        defs.append((key,len(params)));params.append(p)
    add('matrixDest1','Slot 1 Destination',0,13,0,DESTS)
    add('matrixDest2','Slot 2 Destination',0,13,0,DESTS)
    add('matrixAmount2','Slot 2 Amount',-1,1,0,dynamic_display=True)
    for slot in (3,4):
        add(f'matrixSource{slot}',f'Slot {slot} Source',0,7,0,SOURCES)
        add(f'matrixDest{slot}',f'Slot {slot} Destination',0,13,0,DESTS)
        add(f'matrixAmount{slot}',f'Slot {slot} Amount',-1,1,0,dynamic_display=True)
    add('matrixStatus','Matrix Status',0,2,0,['Legacy: LFO only','MODS Off','Active'],live=True)
    add('matrixSource1','Slot 1 Source',0,0,0,display='string',live=True,poll=False)
    add('matrixSource2','Slot 2 Source',0,0,0,display='string',live=True,poll=False)
    add('seqStartMode','Start Mode',0,2,0,['Host Transport','Note Gate/Hold','Note Trigger'])
    add('seqTriggerStatus','Sequencer Start Status',0,4,0,['Sequencer Off','Host Transport','Waiting for Note','Note Gate Running','Note Trigger Running'],live=True)
    add('seqTriggerStop','Stop Note Sequencer',0,1,0,['Off','Stop'],momentary=True,debounce_ms=250)
    path.write_text(json.dumps(data,indent=2)+'\n')
    (ROOT/'params.lock').write_text('\n'.join(p['key'] for p in params)+'\n')
    lines=['#pragma once','namespace matrixparam {',f'constexpr int First={first}, Count=9, End={first+12};']
    lines += [f'constexpr int {key}={index};' for key,index in defs]
    lines+=['}'];(ROOT/'src/MatrixParams.h').write_text('\n'.join(lines)+'\n')
    (ROOT/'src/RunModeParams.h').write_text('#pragma once\nnamespace runparam { constexpr int Mode=600, Status=601, Stop=602; }\n')
