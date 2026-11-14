#!/usr/bin/env python3
"""Deterministic 303 instrument artwork; native MPC handles/actions stay intact.
All panels, rotary frames, switches and captions are generated from the editable
layout. No generated image is a flattened replacement for an interactive control.
"""
import json
import os
import math
from pathlib import Path
import random
import sys
from PIL import Image, ImageDraw, ImageFont

ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(Path(os.environ.get('MPC_VST',ROOT.parent/'mpc-vst-plugins'))/'tools'))
import shadow_skin as shadow
SKIN=ROOT/'build/skin/midilab - VST - JC303 MPC/Plugin Skins'
S=3
PAPER='#B5BAB1'
INK='#202820'
MUTED='#56604F'
GREEN='#C6F35B'
ORANGE='#EE733C'
DARK='#172019'
FONT=ROOT/'art/DejaVuSans.ttf'
TABS,TOP=shadow.parse_layout(ROOT/'layout.conf')
PARAMS={p['key']:p for p in json.loads((ROOT/'params.json').read_text())['params']}


def font(size):return ImageFont.truetype(str(FONT),round(size*S))
def new(size,color):return Image.new('RGBA',(size[0]*S,size[1]*S),color)
def box(draw,rect,fill,outline=None,radius=0,width=1):
    rect=tuple(round(v*S) for v in rect)
    if radius:draw.rounded_rectangle(rect,radius=round(radius*S),fill=fill,outline=outline,width=round(width*S))
    else:draw.rectangle(rect,fill=fill,outline=outline,width=round(width*S))
def line(draw,pts,fill,width=1):draw.line([(round(x*S),round(y*S)) for x,y in pts],fill=fill,width=round(width*S))
def ellipse(draw,rect,fill,outline=None,width=1):draw.ellipse(tuple(round(v*S) for v in rect),fill=fill,outline=outline,width=round(width*S))
def text(draw,xy,caption,size=16,fill=INK,anchor='mm'):
    draw.text((round(xy[0]*S),round(xy[1]*S)),caption,font=font(size),fill=fill,anchor=anchor)
def save(im,path):im.resize((im.width//S,im.height//S),Image.Resampling.LANCZOS).save(path)
def screw(draw,x,y):
    ellipse(draw,(x-5,y-5,x+5,y+5),'#72796E','#D5D9D1')
    line(draw,[(x-2,y+2),(x+2,y-2)],'#333C32',1.5)
def lcd(draw,rect):
    x,y,xx,yy=rect
    box(draw,(x-3,y-3,xx+3,yy+3),'#77816E','#CDD3C6',3)
    box(draw,rect,DARK,'#3B4933',2)
    line(draw,[(x+3,yy-2),(xx-3,yy-2)],'#24321F')
def smiley(draw,x,y,r=27):
    ellipse(draw,(x-r-2,y-r-1,x+r+2,y+r+3),'#080E09')
    ellipse(draw,(x-r,y-r,x+r,y+r),'#F7D848','#A48E24',1.5)
    for dx in (-9,9):ellipse(draw,(x+dx-3,y-11,x+dx+3,y-1),'#192019')
    draw.arc(tuple(round(v*S) for v in (x-17,y-17,x+17,y+17)),start=15,end=165,fill='#192019',width=round(3*S))

def section(draw,x,y,w,title):
    text(draw,(x,y),title,12,MUTED,'lm')
    start=x+ImageFont.truetype(str(FONT),12).getlength(title)+14
    line(draw,[(start,y),(x+w,y)],'#8A9481')

def background(index,tab):
    im=new((1280,628),'#858F80');d=ImageDraw.Draw(im)
    # Subtle brushed anodised aluminium; fixed seed keeps builds reproducible.
    rng=random.Random(303)
    for y in range(628):
        v=round(188-16*y/628+rng.uniform(-1.2,1.2))
        line(d,[(0,y),(1280,y)],(v,v+4,v-5,255))
    box(d,(15,10,1265,617),None,'#7B8672',12,2)
    line(d,[(23,12),(1256,12)],'#E0E3D9')
    for x in (30,1250):
        for y in (25,603):screw(d,x,y)
    box(d,(48,19,1232,101),DARK,'#526145',6)
    text(d,(66,53),'JC303',43,'#F0F2E5','lm')
    text(d,(281,48),'ACID BASS',20,GREEN,'lm')
    text(d,(282,74),'MONOPHONIC SYNTHESIZER',10,'#A4AE99','lm')
    line(d,[(66,87),(245,87)],ORANGE,4)
    line(d,[(252,87),(420,87)],GREEN,4)
    badge_right=1140 if index==0 else 1210
    badge_left=badge_right-240
    text(d,(badge_right-16,49),f'{index+1:02d} / {tab["name"]}',20,'#EEF3E5','rm')
    for n in range(len(TABS)):
        x=badge_right-len(TABS)*16+n*16
        box(d,(x,77,x+11,81),GREEN if n==index else '#45523C',radius=1)
    if index==0:smiley(d,1187,59,28)
    # A printed footer, outside all touch regions.
    text(d,(62,621),'JC / 303',7,MUTED,'lm')
    text(d,(1219,621),'ACID ENGINE',7,MUTED,'rm')
    for w in tab['widgets']:
        k=w['kind']
        if k=='knob':
            x=w['cx'];y=w['cy']-86;r=w['r'];s=2*r+10
            # The value is a native live label over this recessed display.
            name_h=max(23,round(w.get('ns',18)*1.2))
            value_y=y-s//2+s//2+r+2+name_h+2
            lcd(d,(x-101,value_y-1,x+101,value_y+29))
        elif k in ('popup','readout'):
            x=w['cx'];y=w['cy']-86;ww=w['w'];hh=w['h']
            lcd(d,(x-ww//2,y-hh//2,x+ww//2,y+hh//2))
            if w.get('label'):text(d,(x,y-hh//2-17),w['label'],13,INK)
            if k=='popup':
                xx=x+ww//2-20
                d.polygon([(round((xx-7)*S),round((y-3)*S)),(round((xx+7)*S),round((y-3)*S)),(round(xx*S),round((y+5)*S))],fill=GREEN)
        elif k.startswith('enum'):
            w=dict(w);w.setdefault('options',PARAMS[w['key']]['options'])
            top=min(rect[1] for rect in shadow.seg_rects(w))-86
            text(d,(w['cx'],top-17),w.get('label',''),13,INK)
        elif k=='list':
            for number,(x,y,ww,hh) in enumerate(shadow.list_tiles(w)):
                y-=86
                box(d,(x-2,y-2,x+ww+2,y+hh+2),'#77816E','#D3D8CD',3)
                box(d,(x,y,x+ww,y+hh),DARK,radius=2)
    if index==0:
        section(d,74,126,490,'OSCILLATOR')
        section(d,674,126,526,'VOLTAGE CONTROLLED FILTER')
        section(d,74,340,1126,'ENVELOPE / OUTPUT')
        # Printed waveform emblem, a nod to the original 303's silkscreen.
        line(d,[(86,557),(122,557),(122,537),(158,537),(158,557),(194,557)],MUTED,2)
        line(d,[(230,557),(266,537),(266,557),(302,537),(302,557)],MUTED,2)
        text(d,(1200,548),'BASS LINE / ACID MACHINE',13,MUTED,'rm')
    elif index==1:
        section(d,74,126,1126,'CIRCUIT MODIFICATIONS')
        box(d,(684,476,1205,578),None,'#929D86',6)
        text(d,(706,499),'BEYOND THE ORIGINAL',16,INK,'lm')
        text(d,(706,526),'ENVELOPE  /  SLIDE  /  DIODE FILTER',11,MUTED,'lm')
        text(d,(706,553),'ENABLE MODS TO ENGAGE THE EXTENDED CIRCUIT',10,MUTED,'lm')
    elif index==2:
        # Selector labels occupy the upper silkscreen row on this page.
        box(d,(390,355,1200,555),None,'#929D86',8)
        text(d,(420,381),'MODULATION / ACID MOTION',13,MUTED,'lm')
        for n in range(4):line(d,[(420,410+n*37),(1175,410+n*37)],'#A4AE99')
        pts=[(420+x,468-43*math.sin(x/58)) for x in range(756)]
        line(d,pts,'#65784C',2)
        line(d,[(420,505),(570,430),(570,505),(720,430),(720,505),(870,430),(870,505),(1020,430),(1020,505),(1175,430)],'#949F85',1.5)
    elif index==3:
        text(d,(792,128),'16 STEP / HOST SYNC',12,MUTED)
    elif index==4:
        section(d,74,259,1126,'PATTERN GENERATOR')
    elif index==5:
        section(d,74,126,1126,'NEURAL DRIVE / 37 MODELS')
        # Printed signal-flow legend describes the existing audio path.
        for x,label in [(156,'303 VOICE'),(455,'AMP / PEDAL'),(754,'DRY / WET'),(1053,'OUTPUT')]:
            box(d,(x-90,533,x+90,570),None,'#77866A',4)
            text(d,(x,551),label,13,INK)
        for x in (265,564,863):
            line(d,[(x,551),(x+75,551)],MUTED,2)
            line(d,[(x+68,546),(x+75,551),(x+68,556)],MUTED,2)
    elif index==6:
        section(d,74,246,1126,'MODS ON / LEGACY KEEPS THE PREVIOUS SOUND')
    elif index==7:
        section(d,74,198,1126,'ENGINE: JC303 0.14 / MODS ON')
    save(im,SKIN/f'sh_bg_{index}.png')


def knob_strip(radius):
    size=2*radius+10;c=size/2
    strip=Image.new('RGBA',(size,size*128))
    for n in range(128):
        im=new((size,size),(0,0,0,0));d=ImageDraw.Draw(im)
        for i in range(25):
            a=math.radians(135+270*i/24)
            inner,outer=radius-4,radius+2
            line(d,[(c+inner*math.cos(a),c+inner*math.sin(a)),(c+outer*math.cos(a),c+outer*math.sin(a))],GREEN if i<=n*24/127 else '#64715B',1.4)
        cap=radius-10
        ellipse(d,(c-cap-3,c-cap-1,c+cap+4,c+cap+6),(20,28,16,70))
        ellipse(d,(c-cap-1,c-cap-1,c+cap+1,c+cap+1),'#11180F','#626B58')
        for r in range(cap,0,-1):
            v=28+round(20*(cap-r)/cap)
            ellipse(d,(c-r,c-r,c+r,c+r),(v,v+4,v-1,255))
        for i in range(36):
            a=2*math.pi*i/36
            line(d,[(c+(cap-4)*math.cos(a),c+(cap-4)*math.sin(a)),(c+(cap-1)*math.cos(a),c+(cap-1)*math.sin(a))],'#111A10',1)
        ellipse(d,(c-cap+6,c-cap+6,c+cap-6,c+cap-6),'#30372C','#59634F')
        a=math.radians(135+270*n/127)
        line(d,[(c+10*math.cos(a),c+10*math.sin(a)),(c+(cap-4)*math.cos(a),c+(cap-4)*math.sin(a))],'#E5F7C7',3)
        frame=im.resize((size,size),Image.Resampling.LANCZOS)
        strip.paste(frame,(0,n*size))
    strip.save(SKIN/f'sh_knob_r{radius}.png')


def switch_assets():
    for on in (False,True):
        im=new((53,29),(0,0,0,0));d=ImageDraw.Draw(im)
        box(d,(1,2,51,27),'#1B2418','#647358',4)
        box(d,(5,7,47,22),'#0B100A',radius=3)
        x=28 if on else 5
        box(d,(x,5,x+20,24),GREEN if on else '#65705B','#B0C194' if on else '#849078',3)
        for xx in range(x+6,x+16,3):line(d,[(xx,10),(xx,19)],'#738B44' if on else '#3F4B34')
        save(im,SKIN/f'sh_pill_{"on" if on else "off"}.png')


def key_image(path,caption,on,fill=None,small=False):
    with Image.open(path) as original:size=original.size
    im=new(size,(0,0,0,0));d=ImageDraw.Draw(im)
    w,h=size
    box(d,(0,0,w-1,h-1),'#66725B',radius=3)
    background=fill or (GREEN if on else '#263022')
    box(d,(1,1,w-2,h-3),background,'#90A876' if not on else '#E1FF9B',3)
    line(d,[(4,h-2),(w-4,h-2)],'#3F4B35',1)
    if caption:
        for px in range(18 if small else 21,10,-1):
            if font(px).getlength(caption)<(w-12)*S:break
        text(d,(w/2,h/2-1),caption,px,INK if on else '#EBF0DA')
    save(im,path)


for i,tab in enumerate(TABS):background(i,tab)
for r in {w['r'] for tab in TABS for w in tab['widgets'] if w['kind']=='knob'}:knob_strip(r)
switch_assets()
for tab in TABS:
    for w in tab['widgets']:
        if w['kind'].startswith('enum'):
            options=w.get('options',PARAMS[w['key']]['options'])
            for i,caption in enumerate(options):
                for on in (False,True):
                    key_image(SKIN/f'sh_seg_{w["key"]}_{i}_{"on" if on else "off"}.png',caption,on,small=True)
        elif w['kind']=='button':
            prefix=f'sh_btn_{w["key"]}_'
            for path in SKIN.glob(prefix+'*.png'):
                on=path.stem.endswith('_on')
                key_image(path,w['label'],on,fill=ORANGE if on and w['key']=='seqClear' else None)

models=json.loads((ROOT/'vendor/guitarml-models/index.json').read_text())
for i,model in enumerate(models):
    for on in (False,True):
        key_image(SKIN/f'sh_po_{i}_{"on" if on else "off"}.png',model['name'],on,small=True)
# Keep step text on the same dark field in both states. Selection is a border.
for path in SKIN.glob('sh_tile_*_*.png'):
    with Image.open(path) as original:size=original.size
    im=new(size,DARK);d=ImageDraw.Draw(im)
    selected=path.stem.endswith('_on')
    box(d,(1,1,size[0]-2,size[1]-2),None,GREEN if selected else '#66725B',3,3 if selected else 1)
    save(im,path)
# Popup bezel follows the same black/green hardware language.
for path in SKIN.glob('sh_pop_*.png'):
    with Image.open(path) as original:size=original.size
    im=new(size,DARK);d=ImageDraw.Draw(im)
    box(d,(1,1,size[0]-2,size[1]-2),None,GREEN,4,2)
    save(im,path)
print('Acid 303 artwork: six panels, rotary filmstrips, switches, keys and chooser.')

# Q-Link geometry must include the native name/value labels, not just knob art.
import skin_focus
skin_focus.finish(SKIN,TABS,PARAMS)
