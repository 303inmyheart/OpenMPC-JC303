"""Embed vendored built-in GuitarML JSON without runtime files or downloads."""
import json
from pathlib import Path

def generate(root):
    folder=root/'vendor/guitarml-models'
    models=json.loads((folder/'index.json').read_text())
    out=['// Generated from vendored JC303 model JSON; do not edit.',
         '#include "ModelData.h"','namespace jc303drive {']
    for i,model in enumerate(models):
        source=json.loads((folder/model['file']).read_text())
        meta=source['model_data']
        assert meta['hidden_size']==40 and meta['input_size'] in (1,2) and meta['unit_type']=='LSTM'
        assert meta.get('sample_rate',44100)==44100
        text=json.dumps(source,separators=(',',':'))
        out.append(f'static const char model_{i}[]=R"GML({text})GML";')
    out.append('const ModelData models[]={')
    out.extend('{'+json.dumps(m['name'])+','+json.dumps(m['file'])+f',model_{i},'+
               str(json.loads((folder/m['file']).read_text())['model_data']['input_size'])+'},'
               for i,m in enumerate(models))
    out+=['};','const int modelCount=sizeof(models)/sizeof(models[0]);','}']
    path=root/'build/model_data.cpp'
    path.parent.mkdir(exist_ok=True)
    path.write_text('\n'.join(out)+'\n')
    return path
