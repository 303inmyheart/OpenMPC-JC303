"""Project-local Q-Link frame bounds from the actual generated native controls."""
import json
from pathlib import Path


def rect(bounds):return tuple(int(float(x)) for x in bounds['bounds'].split())
def finish(skin,tabs,params):
    path=Path(skin)/'TUI.json'
    data=json.loads(path.read_text());page=data['pageData']
    defs={d['key']:d['value'] for d in page['componentDefinitions']['localComponentDefinitions']}
    ids={key:i for i,key in enumerate(params)}
    for tab,layout in zip(page['tabs'],tabs):
        children=defs[tab['componentName']]['componentsData']
        frames=[]
        keys=layout['qlinks'][0][1]
        for col in range(4):
            boxes=[]
            for key in keys[col*4:col*4+4]:
                if key=='-':continue
                widget=next(w for w in layout['widgets'] if w.get('key')==key)
                target='Parameter '+str(ids[key])
                for child in children:
                    kind=child['componentData']['type']
                    if kind=='Image' or kind.startswith(('shPopPanel_','shPopOpt_')):continue
                    maps=child.get('handle remapping',{}).get('map',[])
                    if not any(m['value']==target for m in maps):continue
                    x,y,w,h=rect(child['bounds']);boxes.append((x,y,x+w,y+h))
                # Printed selector/readout headings are outside their native hit box.
                if widget['kind'].startswith('enum'):
                    import shadow_skin
                    enriched=dict(widget);enriched.setdefault('options',params[key]['options'])
                    for x,y,w,h in shadow_skin.seg_rects(enriched):
                        boxes.append((x,y-86-25,x+w,y-86+h))
                elif widget['kind'] in ('popup','readout') and widget.get('label'):
                    x,y,w,h=widget['cx'],widget['cy']-86,widget['w'],widget['h']
                    boxes.append((x-w//2,y-h//2-25,x+w//2,y+h//2))
            if not boxes:frames.append('0 0 0 0');continue
            left=min(b[0] for b in boxes)-6;top=min(b[1] for b in boxes)-6
            right=max(b[2] for b in boxes)+6;bottom=max(b[3] for b in boxes)+6
            assert 0<=left<right<=1280 and 0<=top<bottom<=628,(tab['tabName'],col,boxes)
            frames.append(f'{left} {top} {right-left} {bottom-top}')
        while frames and frames[-1]=='0 0 0 0':frames.pop()
        tab['qlinkBoundsData']=frames
    path.write_text(json.dumps(data,indent=2)+'\n')
    print('Q-Link frames now include complete native controls and captions.')
