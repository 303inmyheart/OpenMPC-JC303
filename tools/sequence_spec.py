"""Append-only 64-step editor API. Existing indices 0..85 stay frozen."""
from pathlib import Path
import json
ROOT=Path(__file__).resolve().parents[1]
def generate():
    path=ROOT/'params.json';data=json.loads(path.read_text());params=data['params'][:86];defs=[]
    def add(key,name,lo=0,hi=1,default=0,options=None,**kwargs):
        x=dict(key=key,name=name,min=lo,max=hi,default=default,**kwargs)
        if options:x['options']=options
        params.append(x);defs.append((key,len(params)-1));return len(params)-1
    def integer(key,name,lo,hi,default):return add(key,name,lo,hi,default,display='int',dynamic_display=True)
    add('seqTiming','Sequence Timing',0,1,1,['Legacy 16','New 64'])
    add('seqPage','Step Page',0,3,0,['1-16','17-32','33-48','49-64'])
    integer('seqLength64','Pattern Length',1,64,16)
    integer('seqSelected64','Selected Step',1,64,1)
    add('seqSwing','Shuffle',50,75,50,unit='%',dynamic_display=True)
    add('seqGatePct','Step Length',1,100,50,unit='%',dynamic_display=True)
    add('seqScale','Step Scale',0,6,2,['1/64','1/32','1/16','1/8','1/4','1/2','1/1'])
    add('seqModifier','Step Modifier',0,2,0,['Full','Dotted','Triplet'])
    add('seqReversePlay','Reverse Playback',0,1,0,['Off','On'])
    integer('seqPitchLen','Pitch Length',0,64,0)
    integer('seqShift','Pattern Shift',-63,63,0)
    integer('seqPitchRotate','Pitch Rotation',-63,63,0)
    add('seqCpuTiming','303 CPU Timing',0,1,0,['Off','On'])
    for key,name in [('seqMute','Muted Note'),('seqHammer','Hammer'),('seqReverse','Reverse Note'),('seqVibrato','Vibrato')]:add(key,name,0,1,0,['Off','On'])
    for key,name in [('seqUndo','Undo'),('seqRedo','Redo'),('seqCopy','Copy Pattern'),('seqPaste','Paste Pattern'),('seqCopyPage','Copy Page'),('seqPastePage','Paste Page'),('seqClearPage','Clear Page'),('seqShiftLeft','Shift Left'),('seqShiftRight','Shift Right'),('seqTransDown','Transpose Down'),('seqTransUp','Transpose Up')]:
        add(key,name,0,1,0,['Off','Do'],momentary=True,debounce_ms=250)
    add('seqVibRate','Vibrato Rate',1,20,6,unit='Hz',dynamic_display=True)
    add('seqVibDepth','Vibrato Spread',0,2,.35,unit='st',dynamic_display=True)
    for key,name in [('seqLiveGate','Gate Override'),('seqLiveSlide','Slide Override'),('seqLiveAccent','Accent Override'),('seqLiveVibrato','Vibrato Override')]:add(key,name,0,2,0,['Pattern','Off','On'])
    integer('seqOctBase','Base Octave',0,7,3)
    add('seqFollow','Follow Playhead',0,1,0,['Off','On'])
    add('seqInfo','Pattern Info',0,0,0,display='string',get='seqInfo_display',live=True)
    add('seqCanUndo','Undo Available',0,1,0,['No','Yes'],live=True)
    add('seqCanRedo','Redo Available',0,1,0,['No','Yes'],live=True)
    ranges={}
    for kind,label in [('Gate','Gate'),('Slide','Slide'),('Accent','Accent'),('Vibrato','Vibrato'),('Tie','Tie'),('Mute','Mute'),('Hammer','Hammer'),('Reverse','Reverse')]:
        ranges[kind]=len(params)
        for col in range(16):add(f'seqGrid{kind}{col+1}',f'{label} Step {col+1}',0,1,0,['Off','On'],legacy=True)
    ranges['PitchView']=len(params)
    for col in range(16):add(f'seqPitchView{col+1}',f'Pitch View {col+1}',0,12,0,['Rest','C','C#','D','D#','E','F','F#','G','G#','A','A#','B'],live=True,legacy=True)
    ranges['OctView']=len(params)
    for col in range(16):add(f'seqOctView{col+1}',f'Octave View {col+1}',0,4,0,['Base','+3','+2','+1','-1'],live=True,legacy=True)
    ranges['PlayView']=len(params)
    for col in range(16):add(f'seqPlayView{col+1}',f'Playhead {col+1}',0,1,0,['Off','On'],live=True,legacy=True)
    ranges['NoteAction']=len(params)
    for col in range(16):
        for pitch in range(12):add(f'seqCell{col+1}_{pitch}',f'Note Cell {col+1}/{pitch}',0,1,0,['Off','Tap'],momentary=True,debounce_ms=250,legacy=True)
    ranges['OctAction']=len(params)
    for col in range(16):
        for row in range(4):add(f'seqOctCell{col+1}_{row}',f'Octave Cell {col+1}/{row}',0,1,0,['Off','Tap'],momentary=True,debounce_ms=250,legacy=True)
    ranges['SelectedView']=len(params)
    for col in range(16):add(f'seqSelectedView{col+1}',f'Selected Column {col+1}',0,1,0,['Off','On'],live=True,legacy=True)
    path.write_text(json.dumps(data|{'params':params},indent=2)+'\n')
    (ROOT/'params.lock').write_text('\n'.join(p['key'] for p in params)+'\n')
    lines=['#pragma once','namespace seqparam {',f'constexpr int First=86, End={len(params)};']
    for k,i in defs:lines.append(f'constexpr int {k}={i};')
    for k,i in ranges.items():lines.append(f'constexpr int {k}First={i};')
    lines+=['}'];(ROOT/'src/sequencer/SeqParams.h').write_text('\n'.join(lines)+'\n')
if __name__=='__main__':generate()
