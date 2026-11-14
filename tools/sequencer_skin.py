#!/usr/bin/env python3
"""Original Acid piano roll built from native MPC buttons and indexed images.
Pitch/octave taps are write-only actions; read-only views display canonical state.
"""
import json,os,sys
from pathlib import Path
from copy import deepcopy
from PIL import Image,ImageDraw,ImageFont
ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(Path(os.environ.get('MPC_VST',ROOT.parent/'mpc-vst-plugins'))/'tools'))
import shadow_skin as s
skin=ROOT/'build/skin/midilab - VST - JC303 MPC/Plugin Skins'
p=skin/'TUI.json';doc=json.loads(p.read_text());pd=doc['pageData'];defs=pd['componentDefinitions']['localComponentDefinitions']
lookup={d['key']:d['value'] for d in defs}
params=json.loads((ROOT/'params.json').read_text())['params'];ids={q['key']:i for i,q in enumerate(params)}
FONT=ROOT/'art/DejaVuSans.ttf'
S=2
GREEN='#C6F35B';ORANGE='#EE733C';DARK='#172019'
font=lambda px:ImageFont.truetype(str(FONT),px*S)
def text(d,xy,t,px=12,ink=GREEN):d.text(tuple(v*S for v in xy),t,font=font(px),fill=ink,anchor='mm')
def box(d,r,fill,outline=None,width=1):d.rectangle(tuple(v*S for v in r),fill=fill,outline=outline,width=width*S)
def asset(name,size,fill,outline=None):
    im=Image.new('RGBA',(size[0]*S,size[1]*S),fill);d=ImageDraw.Draw(im)
    if outline:box(d,(1,1,size[0]-2,size[1]-2),None,outline,2)
    im.resize(size,Image.Resampling.LANCZOS).save(skin/name)
def image(name,x,y,w,h,condition=None):
    c=s._sub('Image',{'version':2,'imageType':'Regular','colour':'0','image':name},s._bounds(x,y,w,h),'Artwork')
    if condition:c['bounds']['additionalInvalidatingHandles']=[condition]
    return c

def button_def(name,w,h,on,off):
    # Persistent switches use the same recipe as the working MODS toggle:
    # Button owns the tap, parent Mouse Down only selects its Q-Link bank.
    # Momentary cells use the trigger recipe and its engine-side debounce.
    persistent=name=='seqFlag'
    actions=[s._action('Mouse Down','Q-Link' if persistent else 'Toggle Switch'),s._action('Enter Pressed','Toggle Switch')]
    children=([s._focus(w,h)] if persistent else [])+[s._button(on,off,1,1,w,h)]
    definition=s._local(name,actions,children)
    definition['value']['hideQLinkBounds']=False
    defs.append(definition)
def put(target,kind,key,x,y,w,h):target.append(s._placed(kind,params[ids[key]]['name'],ids[key],x,y+86,w,h,focus='Yes' if kind=='seqFlag' else 'No'))
# Transparent nonzero alpha remains hit-testable on native MPC.
asset('seq_hit.png',(65,20),(0,0,0,1))
asset('seq_note.png',(65,20),GREEN,'#F0FFCF')
asset('seq_oct.png',(65,22),ORANGE,'#FFE0B1')
asset('seq_flag_off.png',(65,26),'#222D20','#53604A')
asset('seq_flag_on.png',(65,26),GREEN,'#F0FFCF')
asset('seq_play.png',(65,22),ORANGE,'#FFE0B1')
asset('seq_running.png',(2,458),ORANGE)
button_def('seqNoteTap',65,20,'seq_hit.png','seq_hit.png')
# Each definition owns its exact hit rectangle.
asset('seq_oct_hit.png',(65,22),(0,0,0,1));button_def('seqOctTap',65,22,'seq_oct_hit.png','seq_oct_hit.png')
button_def('seqFlag',65,26,'seq_flag_on.png','seq_flag_off.png')
X=112;W=67
main=lookup[next(t['componentName'] for t in pd['tabs'] if t['tabName']=='SEQUENCE')]
children=main['componentsData']
# Compact header control: value beside the knob, above the step-number row.
length=next(c for c in children if c['componentData']['type'].startswith('shKnob'))
length['bounds']['bounds']='580 66 112 60'
lengthDef=lookup[length['componentData']['type']]
for child in lengthDef['componentsData']:
    kind=child['componentData']['type']
    if kind=='Focus':child['bounds']['bounds']='0 0 112 60'
    elif kind=='Knob':child['bounds']['bounds']='5 0 46 46'
    elif kind=='Label':
        name=child['componentData']['data']['type']=='Name'
        child['bounds']['bounds']='56 0 56 22' if name else '56 22 56 30'
        child['componentData']['data']['textStyle']['colour']='ff202820'
mainTab=next(t for t in pd['tabs'] if t['tabName']=='SEQUENCE')
# Final navigation derives the sole length-knob frame from these native bounds.

# Compact original hardware panel; native header controls above the grid remain.
im=Image.new('RGBA',(1280*S,628*S),'#AFB7A7');d=ImageDraw.Draw(im)
box(d,(16,10,1264,615),None,'#6C785F',2);box(d,(30,18,1250,55),DARK)
text(d,(135,36),'JC303 / ACID',22);text(d,(660,36),'64 STEP PIANO ROLL',18);text(d,(1118,36),'SEQUENCE / PIANO',14)
box(d,(699,77,889,113),DARK,'#687A58')
box(d,(95,126,1194,592),DARK)
for col in range(16):
    x=X+col*W
    box(d,(x,130,x+65,152),'#34432D','#687A58')
    # Page-relative positions are intentionally labelled 1..16; page header gives absolute range.
    text(d,(x+32,141),str(col+1),13,'#ECF1DF')
    for row in range(4):box(d,(x,156+row*22,x+65,176+row*22),'#273122','#4C5941')
    for pitch in range(12):
        y=250+(11-pitch)*20
        box(d,(x,y,x+65,y+18),'#202B1D' if pitch in (1,3,6,8,10) else '#34402D','#4C5941')
for row,label in enumerate(('+3','+2','+1','-1')):text(d,(73,167+row*22),label,14,DARK)
for pitch,label in enumerate(('C','C#','D','D#','E','F','F#','G','G#','A','A#','B')):text(d,(72,260+(11-pitch)*20),label,13,DARK)
for y,label in ((500,'SLIDE'),(530,'ACCENT'),(560,'VIBRATO')):text(d,(60,y+13),label,12,DARK)
text(d,(640,605),'TAP A NOTE TO ENTER / TAP AGAIN FOR REST    |    PIANO / ARTIC / TOOLS / LIVE / KEY / GENERATE',12,DARK)
im.resize((1280,628),Image.Resampling.LANCZOS).save(skin/'sh_bg_3.png')
# Scale rows are display-only backgrounds. Inputs and entered notes are above them.
asset('seq_scale_row.png',(1072,19),(198,243,91,55))
asset('seq_scale_dot.png',(8,8),GREEN)
for pitch in range(12):
    y=250+(11-pitch)*20
    condition=f'IndexedEnabling/1/2/Parameter {ids[f"seqScaleView{pitch}"]}'
    children.append(image('seq_scale_row.png',112,y,1072,19,condition))
    children.append(image('seq_scale_dot.png',91,y+6,8,8,condition))
asset('seq_selected.png',(67,461),(0,0,0,0),GREEN)
for col in range(16):
    children.append(image('seq_selected.png',X+col*W-1,128,67,461,f'IndexedEnabling/1/2/Parameter {ids[f"seqSelectedView{col+1}"]}'))
# Read-only current playhead above the touch cells; native text stays legible.
for col in range(16):
    x=X+col*W
    condition=f'IndexedEnabling/1/2/Parameter {ids[f"seqPlayView{col+1}"]}'
    children.append(image('seq_play.png',x,130,65,22,condition))
    children.append(image('seq_running.png',x-2,130,2,458,condition))
    for row in range(4):
        y=156+row*22
        children.append(image('seq_oct.png',x,y,65,22,f'IndexedEnabling/{row+1}/5/Parameter {ids[f"seqOctView{col+1}"]}'))
        put(children,'seqOctTap',f'seqOctCell{col+1}_{row}',x,y,65,22)
    for pitch in range(12):
        y=250+(11-pitch)*20
        children.append(image('seq_note.png',x,y,65,20,f'IndexedEnabling/{pitch+1}/13/Parameter {ids[f"seqPitchView{col+1}"]}'))
        put(children,'seqNoteTap',f'seqCell{col+1}_{pitch}',x,y,65,20)
    for y,kind in ((500,'Slide'),(530,'Accent'),(560,'Vibrato')):
        put(children,'seqFlag',f'seqGrid{kind}{col+1}',x,y,65,26)
# Keep each step number readable above the active orange playhead.
for col in range(16):
    x=X+col*W
    # Caption rendered into transparent art. It does not cover note input regions.
    name=f'seq_number_{col+1}.png';im=Image.new('RGBA',(65*S,22*S));text(ImageDraw.Draw(im),(32,11),str(col+1),13,'#ECF1DF');im.resize((65,22),Image.Resampling.LANCZOS).save(skin/name)
    children.append(image(name,x,130,65,22))
# Articulation grid: all step attributes, with note controls kept above it.
art=lookup[next(t['componentName'] for t in pd['tabs'] if t['tabName']=='ARTIC')]['componentsData']
asset('seq_art_selected.png',(67,283),(0,0,0,0),GREEN)
for col in range(16):
    art.append(image('seq_art_selected.png',X+col*W-1,262,67,283,f'IndexedEnabling/1/2/Parameter {ids[f"seqSelectedView{col+1}"]}'))
for col in range(16):
    x=X+col*W
    for row,kind in enumerate(('Gate','Slide','Accent','Vibrato','Tie','Mute','Hammer','Reverse')):
        put(art,'seqFlag',f'seqGrid{kind}{col+1}',x,285+row*32,65,26)
# Print labels on ARTIC background, below existing knobs and captions.
with Image.open(skin/'sh_bg_8.png') as original:im=original.convert('RGBA').resize((1280*S,628*S))
d=ImageDraw.Draw(im)
for col in range(16):text(d,(X+col*W+32,269),str(col+1),13,DARK)
for row,label in enumerate(('GATE','SLIDE','ACCENT','VIBRATO','TIE','MUTE','HAMMER','REVERSE')):text(d,(58,298+row*32),label,11,DARK)
text(d,(640,581),'MUTE EXCLUDES ACCENT  /  HAMMER EXCLUDES SLIDE  /  TIE EXTENDS THE PREVIOUS NOTE',12,DARK)
im.resize((1280,628),Image.Resampling.LANCZOS).save(skin/'sh_bg_8.png')
p.write_text(json.dumps(doc,indent=2)+'\n')
print('Native 64-step piano roll: 192 note cells, 64 octave cells, three flag lanes, articulation grid.')
