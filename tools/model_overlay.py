#!/usr/bin/env python3
"""Put the drive picker in a real native MPC overlay, above page Q-Link bounds.
Only known native primitives are used: KnobOverlay supplies the stock overlay
Focus Lost dismissal behavior; our own images/radio buttons draw the picker.
No Akai artwork is copied. The reserved legacy __open parameter stays in place.
"""
import copy
import json
from pathlib import Path
from PIL import Image,ImageDraw,ImageFont
ROOT=Path(__file__).resolve().parents[1]
cfg=json.loads((ROOT/'vst.json').read_text())
SKIN=ROOT/'build/skin'/f"{cfg['vendor']} - VST - {cfg['name']}"/'Plugin Skins'
NAME='jc303 model overlay'

def bounds(x,y,w,h):
    return {'version':2,'acceptsHWFocus':'No','showWhenDataModelInvalid':'Show',
            'whenVisible':'Always','boundsType':'Absolute','bounds':f'{x} {y} {w} {h}',
            'additionalInvalidatingHandles':[]}
def entry(kind,name,data,x,y,w,h):
    return {'version':2,'componentData':{'version':1,'name':name,'type':kind,'data':data},
            'handle remapping':{'version':1,'map':[]},'bounds':bounds(x,y,w,h)}
def action(event,handler,handle='',extra=''):
    return {'version':2,'onAction':event,'handler':handler,'handleName':handle,
            'additionalData':extra,'handle remapping':{'version':1,'map':[]}}

def install():
    path=SKIN/'TUI.json';doc=json.loads(path.read_text());pd=doc['pageData']
    defs={d['key']:d['value'] for d in pd['componentDefinitions']['localComponentDefinitions']}
    tab=next(t for t in pd['tabs'] if t['tabName']=='DRIVE');page=defs[tab['componentName']]
    parts=[c for c in page['componentsData'] if c['componentData']['type'].startswith(('shPopPanel_','shPopOpt_'))]
    assert len(parts)==38
    panel=next(c for c in parts if c['componentData']['type'].startswith('shPopPanel_'))
    px,py,pw,ph=map(int,panel['bounds']['bounds'].split())
    origin_x=(1280-pw)//2;origin_y=124
    assert origin_y+ph<720
    field=next(c for c in page['componentsData'] if c['componentData']['type'].startswith('shPopField_'))
    model=next(m['value'] for m in field['handle remapping']['map'] if m['key']=='Text')
    field['handle remapping']['map']=[{'key':'Data','value':model},{'key':'Text','value':model}]
    defs[field['componentData']['type']]['actions']=[
        action('Mouse Down','Show Overlay','Data',NAME),
        action('Enter Pressed','Show Overlay','Data',NAME)]
    page['componentsData']=[c for c in page['componentsData'] if c not in parts]
    # The real native overlay owns focus/dismissal, just like the stock knob overlay.
    native=entry('KnobOverlay','Overlay focus and dismissal',{
        'version':1,'actions':[{'version':1,'onAction':'Focus Lost','handler':'Focus Lost',
                               'handleName':'Focus Lost','additionalData':''}],
        'overlayType':'Knob With Header','scaleType':'Linear','polarityType':'Unipolar',
        'showMinusSign':True,'handleName':'Data'},0,0,1280,800)
    # Paint the full-window backdrop as a component BACKGROUND, not as an
    # Image child. A full-screen Image child consumes outside taps before the
    # native KnobOverlay can emit Focus Lost and dismiss the window.
    children=[native]
    for original in parts:
        c=copy.deepcopy(original);x,y,w,h=map(int,c['bounds']['bounds'].split())
        c['bounds']=bounds(origin_x+x-px,origin_y+y-py,w,h)
        # Data is inherited from the native Show Overlay action (the model enum).
        c['handle remapping']['map']=[]
        children.append(c)
    clear={'version':1,'colour':'0','image':'sh_model_overlay.png'}
    overlay={'version':4,'actions':[],'backgroundData':{'version':1,'focussed':clear,'unfocussed':clear},
             'ignoreMousePresses':True,'disableCoarseDataWheel':False,'repeats':1,
             'hideQLinkBounds':True,'componentsData':children}
    pd['componentDefinitions']['localComponentDefinitions'].append({'key':NAME,'value':overlay})
    path.write_text(json.dumps(doc,indent=2)+'\n')
    # Original artwork, at 2x resolution; does not include the native knob's art.
    scale=2;im=Image.new('RGB',(1280*scale,800*scale),'#111A13');d=ImageDraw.Draw(im)
    font=lambda px:ImageFont.truetype(str(ROOT/'art/DejaVuSans.ttf'),px*scale)
    d.text((60*scale,48*scale),'JC303 / DRIVE MODELS',font=font(28),fill='#C6F35B',anchor='lm')
    d.text((60*scale,86*scale),'37 NEURAL AMP / PEDAL MODELS',font=font(14),fill='#ADB99F',anchor='lm')
    d.line([(60*scale,104*scale),(1220*scale,104*scale)],fill='#5D704C',width=2*scale)
    d.text((640*scale,746*scale),'SELECT A MODEL / TAP OUTSIDE THE LIST TO CLOSE',font=font(17),fill='#C6F35B',anchor='mm')
    im.resize((1280,800),Image.Resampling.LANCZOS).save(SKIN/'sh_model_overlay.png')
    # Documentation preview of our overlay contents (without the native host backdrop).
    preview=im.resize((1280,800),Image.Resampling.LANCZOS).convert('RGBA')
    for c in children[1:]:
        definition=defs[c['componentData']['type']]
        x,y,w,h=map(int,c['bounds']['bounds'].split())
        for part in definition['componentsData']:
            cd=part['componentData'];sx,sy,sw,sh=map(int,part['bounds']['bounds'].split())
            image=cd['data'].get('image') if cd['type']=='Image' else cd['data'].get('offImage') if cd['type']=='Button' else None
            if image:preview.alpha_composite(Image.open(SKIN/image).convert('RGBA'),(x+sx,y+sy))
    preview.convert('RGB').save(ROOT/'build/preview_model_overlay.png')
    print('Model picker: native fullscreen overlay; normal page Q-Link rectangles preserved.')

if __name__=='__main__':install()
