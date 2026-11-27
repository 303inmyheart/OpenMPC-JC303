#!/usr/bin/env python3
"""Add a touch-only, native information overlay without changing DSP/state IDs."""
import json
from pathlib import Path
from PIL import Image, ImageDraw, ImageFont
from model_overlay import bounds, entry, action

ROOT=Path(__file__).resolve().parents[1]
CFG=json.loads((ROOT/'vst.json').read_text())
SKIN=ROOT/'build/skin'/f"{CFG['vendor']} - VST - {CFG['name']}"/'Plugin Skins'
NAME='jc303 preset compatibility help'
BUTTON='jc303 preset help button'
SCALE=2

def canvas(w,h,colour):
    im=Image.new('RGB',(w*SCALE,h*SCALE),colour)
    return im,ImageDraw.Draw(im)

def text(draw,x,y,value,size,colour='#EAF0E2'):
    font=ImageFont.truetype(str(ROOT/'art/DejaVuSans.ttf'),size*SCALE)
    box=draw.textbbox((0,0),value,font=font)
    assert x*SCALE+box[2] <= draw._image.width-20*SCALE,(value,'text overflow')
    draw.text((x*SCALE,y*SCALE),value,font=font,fill=colour)

def save(im,name):
    im.resize((im.width//SCALE,im.height//SCALE),Image.Resampling.LANCZOS).save(SKIN/name)

def install():
    path=SKIN/'TUI.json';doc=json.loads(path.read_text());pd=doc['pageData']
    definitions=pd['componentDefinitions']['localComponentDefinitions']
    definitions[:]=[d for d in definitions if d['key'] not in (NAME,BUTTON)]
    defs={d['key']:d['value'] for d in definitions}
    qlinks=(SKIN/'Q-Links.json').read_bytes()

    im,draw=canvas(270,56,'#C6F35B')
    draw.rounded_rectangle((2,2,538,110),radius=12,outline='#F5FFD8',width=4)
    text(draw,34,3,'PRESET INFO',25,'#152015')
    text(draw,26,34,'OLD PRESETS? READ THIS',14,'#24351C')
    save(im,'jc303_preset_help_button.png')
    image=entry('Image','Preset help artwork',{'version':2,'imageType':'Regular',
                'colour':'0','image':'jc303_preset_help_button.png'},0,0,270,56)
    clear={'version':1,'colour':'0','image':''}
    definitions.append({'key':BUTTON,'value':{
        'version':4,'actions':[action('Mouse Down','Show Overlay','Data',NAME)],
        'backgroundData':{'version':1,'focussed':clear,'unfocussed':clear},
        'ignoreMousePresses':False,'disableCoarseDataWheel':False,
        'repeats':1,'hideQLinkBounds':True,'componentsData':[image]}})
    for tab in pd['tabs']:
        children=defs[tab['componentName']]['componentsData']
        children[:]=[c for c in children if c['componentData']['type']!=BUTTON]
        button=entry(BUTTON,'Read before loading old presets',{'version':1,'handleName':'Data'},484,31,270,56)
        # Read-only status provides a valid native overlay model. It cannot
        # change sound settings, and the button is never assigned a Q-Link.
        button['handle remapping']['map']=[{'key':'Data','value':'Parameter 597'}]
        children.append(button)

    im,draw=canvas(1120,680,'#172019')
    draw.rounded_rectangle((3,3,2236,1356),radius=22,outline='#C6F35B',width=4)
    lines=[
        (30,20,"CAN'T LOAD AN OLDER PRESET?",30,'#C6F35B'),
        (30,70,'The maker name changed from midilab to 303inmyheart in this update.',22,'#EAF0E2'),
        (30,100,'An older preset may still contain the old name, so MPC refuses to load it.',22,'#EAF0E2'),
        (30,130,'Your sound may still be intact. Updating the plugin does not convert these files.',21,'#FFAC78'),
        (30,179,'1  FIRST: KEEP YOUR ORIGINALS SAFE',23,'#C6F35B'),
        (60,212,'Back up your presets and projects before trying either option below.',21,'#EAF0E2'),
        (60,241,'Never overwrite the old preset with Init or another sound.',21,'#EAF0E2'),
        (30,289,'2  EASIEST OPTION: THE SOUND IS IN AN OLDER PROJECT',23,'#C6F35B'),
        (60,322,"Open that project. If the sound plays correctly, use MPC's preset Save command.",21,'#EAF0E2'),
        (60,351,'Save under a NEW name. Try loading it in a fresh JC303 instance.',21,'#EAF0E2'),
        (60,380,'This creates a preset for the new version without editing any files.',21,'#EAF0E2'),
        (30,428,'3  ONLY HAVE THE OLD PRESET FILE?',23,'#C6F35B'),
        (60,461,'The .xpl file is the small file MPC created when you saved your preset.',21,'#EAF0E2'),
        (60,490,'A COPY needs its plugin details updated on a computer. This is not a synth setting.',21,'#EAF0E2'),
        (60,519,'Only the maker name and plugin location change; the saved sound data must stay.',21,'#EAF0E2'),
        (60,548,'If you are unsure, keep the original and ask for help. Do not edit the sound data.',21,'#FFAC78'),
        (30,596,'Full step-by-step file instructions: PRESET-MIGRATION.md',21,'#C6F35B'),
        (30,628,'github.com/303inmyheart/OpenMPC-JC303  -  no automatic conversion in this window',20,'#D3DDC8'),
    ]
    for x,y,caption,size,colour in lines:text(draw,x,y,caption,size,colour)
    save(im,'jc303_preset_help_panel.png')
    # Same native focus/dismissal primitive as the device-tested Drive picker.
    # The panel is a partial-size Image: outside taps reach KnobOverlay rather
    # than being swallowed by a full-screen child Image.
    native=entry('KnobOverlay','Native overlay dismissal',{
        'version':1,'actions':[{'version':1,'onAction':'Focus Lost',
            'handler':'Focus Lost','handleName':'Focus Lost','additionalData':''}],
        'overlayType':'Knob With Header','scaleType':'Linear','polarityType':'Unipolar',
        'showMinusSign':True,'handleName':'Data'},0,0,1280,800)
    panel=entry('Image','Preset migration instructions',{'version':2,'imageType':'Regular',
        'colour':'0','image':'jc303_preset_help_panel.png'},80,40,1120,680)
    backdrop,draw=canvas(1280,800,'#080D09')
    text(draw,427,748,'TAP OUTSIDE TO CLOSE',25,'#C6F35B')
    save(backdrop,'jc303_preset_help_backdrop.png')
    background={'version':1,'colour':'0','image':'jc303_preset_help_backdrop.png'}
    definitions.append({'key':NAME,'value':{'version':4,'actions':[],
        'backgroundData':{'version':1,'focussed':background,'unfocussed':background},
        'ignoreMousePresses':True,'disableCoarseDataWheel':True,'repeats':1,
        'hideQLinkBounds':True,'componentsData':[native,panel]}})
    path.write_text(json.dumps(doc,indent=2)+'\n')
    assert (SKIN/'Q-Links.json').read_bytes()==qlinks
    preview=backdrop.resize((1280,800),Image.Resampling.LANCZOS)
    with Image.open(SKIN/'jc303_preset_help_panel.png') as p:preview.paste(p,(80,40))
    preview.save(ROOT/'build/preview_preset_help.png')
    print('Preset Info: touch-only header button on every page; English plain-language native overlay; Q-Links unchanged. Device dismissal test pending.')

if __name__=='__main__':install()
