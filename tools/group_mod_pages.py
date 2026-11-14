#!/usr/bin/env python3
"""Place ENGINE/FILTER under MODS without changing controls or their Q-Link banks."""
from pathlib import Path
from copy import deepcopy
import json
root=Path(__file__).resolve().parents[1]
skin=next((root/'build/skin').glob('*/Plugin Skins/TUI.json')).parent
p=skin/'TUI.json';d=json.loads(p.read_text());page=d['pageData']
original_definitions=deepcopy(page['componentDefinitions'])
mapping={}
main=next(t['fnKeyIndex'] for t in page['tabs'] if t['tabName']=='MODS')
for parent,names in [('MODS',('ENGINE','FILTER')),('SEQUENCE',('ARTIC','SEQTOOLS','SEQLIVE','SEQKEY'))]:
    mainIndex=next(t['fnKeyIndex'] for t in page['tabs'] if t['tabName']==parent)
    for sub,name in enumerate(names,1):
        tab=next(t for t in page['tabs'] if t['tabName']==name)
        mapping[(tab['fnKeyIndex']+1,tab['fnKeySubIndex']+1)]=(mainIndex+1,sub+1)
        tab['fnKeyIndex']=mainIndex;tab['fnKeySubIndex']=sub
page['tabs'].sort(key=lambda t:(t['fnKeyIndex'],t['fnKeySubIndex']))
qpath=skin/'Q-Links.json';q=json.loads(qpath.read_text())
for bank in q['Screen Mode Q-Links']['map']:
    key=(bank['Tab'],bank['SubTab'])
    if key in mapping:bank['Tab'],bank['SubTab']=mapping[key]
q['Screen Mode Q-Links']['map'].sort(key=lambda b:(b['Tab'],b['SubTab']))
pairs=[(b['Tab'],b['SubTab']) for b in q['Screen Mode Q-Links']['map']]
assert len(pairs)==len(set(pairs))
assert set(pairs)=={(t['fnKeyIndex']+1,t['fnKeySubIndex']+1) for t in page['tabs']}
assert set(t['fnKeyIndex'] for t in page['tabs'])==set(range(6))
assert [(t['fnKeySubIndex'],t['tabName']) for t in page['tabs'] if t['fnKeyIndex']==main]==[(0,'MODS'),(1,'ENGINE'),(2,'FILTER')]
assert original_definitions==page['componentDefinitions']
p.write_text(json.dumps(d,indent=2)+'\n');qpath.write_text(json.dumps(q,indent=2)+'\n')
print('MODS -> MODS / ENGINE / FILTER: native subpages and matching Q-Link banks; six main tabs; controls unchanged.')
