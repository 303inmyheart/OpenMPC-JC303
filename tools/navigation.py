"""Single source of truth for native function keys and meaningful Q-Link banks."""
GROUPS=(('SYNTH',('SYNTH',)),('MODS',('MODS','ENGINE','FILTER','LEGACY')),
        ('LFO / MATRIX',('LFO','MATRIX1','MATRIX2','MATRIX3','MATRIX4')),
        ('SEQUENCE',('SEQUENCE','ARTIC','SEQTOOLS','SEQLIVE','SEQKEY','GENERATE')),
        ('DRIVE',('DRIVE',)),('SETUP',('SETUP',)))
LOCATION={name:(main,sub,title) for main,(title,names) in enumerate(GROUPS) for sub,name in enumerate(names)}
def banks(tab):
    knobs=sorted((w for w in tab['widgets'] if w['kind']=='knob'),key=lambda w:(w['cy'],w['cx']))
    if tab['name']=='DRIVE':
        # Explicit user exception: model chooser stays on Q-Links.
        return [[next(w for w in tab['widgets'] if w.get('key')=='driveModel'),*knobs]]
    if not knobs:return []
    if len(knobs)<=4:return [knobs]
    rows={}
    for w in knobs:rows.setdefault(w['cy'],[]).append(w)
    result=[]
    for row in rows.values():
        for first in range(0,len(row),4):result.append(row[first:first+4])
    assert len(result)<=4,(tab['name'],'split crowded knob page')
    return result
