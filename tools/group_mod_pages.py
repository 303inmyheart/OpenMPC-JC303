#!/usr/bin/env python3
"""Apply navigation and knob-only, RowTopDown Q-Link banks to final native controls."""
from pathlib import Path
import os,sys,json
from copy import deepcopy
from navigation import GROUPS,LOCATION,banks
root=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(Path(os.environ.get('MPC_VST',root.parent/'mpc-vst-plugins'))/'tools'))
import shadow_skin
layouts,_=shadow_skin.parse_layout(root/'layout.conf');layout={t['name']:t for t in layouts}
params=json.loads((root/'params.json').read_text())['params'];ids={p['key']:i for i,p in enumerate(params)}
skin=next((root/'build/skin').glob('*/Plugin Skins/TUI.json')).parent
path=skin/'TUI.json';doc=json.loads(path.read_text());page=doc['pageData']
original=deepcopy(page['componentDefinitions'])
defs={d['key']:d['value'] for d in page['componentDefinitions']['localComponentDefinitions']}
qpath=skin/'Q-Links.json';q=json.loads(qpath.read_text());maps=[]
for tab in page['tabs']:
    name=tab['tabName'];main,sub,_=LOCATION[name];tab['fnKeyIndex']=main;tab['fnKeySubIndex']=sub
    children=defs[tab['componentName']]['componentsData'];frames=[];assign={f'Q-Link {i}':-1 for i in range(1,17)}
    groups=banks(layout[name]);assert all(len(g)<=4 for g in groups)
    for bank,controls in enumerate(groups):
        boxes=[]
        for slot,w in enumerate(controls):
            key=w['key'];target=f'Parameter {ids[key]}'
            assert w['kind']=='knob' or (name=='DRIVE' and key=='driveModel')
            assign[f'Q-Link {13+slot-4*bank}']=ids[key]
            bound=[]
            for c in children:
                if c['componentData']['type']=='Image':continue
                if any(m['value']==target for m in c.get('handle remapping',{}).get('map',[])):
                    x,y,ww,hh=map(int,c['bounds']['bounds'].split());bound.append((x,y,x+ww,y+hh))
            assert bound,(name,key,'mapped control absent')
            boxes+=bound
            if w['kind']=='popup' and w.get('label'):
                x,y,ww,hh=w['cx'],w['cy']-86,w['w'],w['h'];boxes.append((x-ww//2,y-hh//2-25,x+ww//2,y+hh//2))
        left=min(b[0] for b in boxes)-6;top=min(b[1] for b in boxes)-6
        right=max(b[2] for b in boxes)+6;bottom=max(b[3] for b in boxes)+6
        assert 0<=left<right<=1280 and 0<=top<bottom<=628,(name,bank,boxes)
        frames.append(f'{left} {top} {right-left} {bottom-top}')
    tab['qlinkBoundsData']=frames or ['0 0 0 0']
    maps.append({'Tab':main+1,'SubTab':sub+1,'Bank Direction':'RowTopDown','Q-Links':assign})
page['tabs'].sort(key=lambda t:(t['fnKeyIndex'],t['fnKeySubIndex']));maps.sort(key=lambda b:(b['Tab'],b['SubTab']))
q['Screen Mode Q-Links']['map']=maps
# Program mode must not retain the generator's first-16 map (contains switches).
program=q.get('Program Mode Q-Links')
if program is not None:
    target=program.get('map',program)
    if isinstance(target,dict):
        for k in target:
            if k.startswith('Q-Link '):target[k]=-1
        for bank,controls in enumerate(banks(layout['SYNTH'])):
            for slot,w in enumerate(controls):target[f'Q-Link {13+slot-4*bank}']=ids[w['key']]
assert original==page['componentDefinitions'],'navigation must not alter working native touch actions'
assert set(layout)==set(LOCATION)
assert {(b['Tab'],b['SubTab']) for b in maps}=={(t['fnKeyIndex']+1,t['fnKeySubIndex']+1) for t in page['tabs']}
path.write_text(json.dumps(doc,indent=2)+'\n');qpath.write_text(json.dumps(q,indent=2)+'\n')
print('Six logical function keys; knob-only Q-Links plus Drive model; RowTopDown banks and final native label/value bounds.')
