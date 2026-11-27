"""Compile upstream v0.14 sound settings into immutable MPC program data.

No runtime JSON parsing, filesystem access or allocations on program lookup.
Parameter 64 and its twelve historical choices are deliberately unchanged.
"""
import hashlib
import json
from pathlib import Path

REVISION = 'b0ff26011f80e45c7796cad7b29fbc7f347b9337'
ALIASES = {
    'switchOverdriveState': 'driveEnabled', 'overdriveModelIndex': 'driveModel',
    'overdriveLevel': 'driveLevel', 'overdriveDryWet': 'driveMix',
    'overdrivePreVca': 'drivePreVca', 'overdriveDisperse': 'driveDisperse',
    'filterType': 'filterModel014', 'oscModel': 'oscModel014',
    'junoWave': 'junoWave014', 'superDetune': 'superDetune014',
    'superMix': 'superMix014', 'shaperMode': 'shaperMode014',
    'shapeDrift': 'shaperDrift014', 'shaperSplit': 'shaperSplit014',
    'squareModel': 'squareModel014', 'filterDrive': 'filterSaturation014',
    'filterMorph': 'filterMorph014', 'filterQuality': 'filterQuality014',
    'filterTension': 'filterTension014', 'filterBias': 'filterBias014',
    'filterRail': 'filterRail014', 'filterFm': 'filterFm014', 'resTrim': 'resTrim014',
    'subOscGain': 'subGain014', 'subOscBlend': 'subBlend014',
    'lfoDestination': 'matrixDest1', 'modSlot2Dest': 'matrixDest2',
    'modSlot2Amount': 'matrixAmount2', 'modSlot3Source': 'matrixSource3',
    'modSlot3Dest': 'matrixDest3', 'modSlot3Amount': 'matrixAmount3',
    'modSlot4Source': 'matrixSource4', 'modSlot4Dest': 'matrixDest4',
    'modSlot4Amount': 'matrixAmount4', 'lfoPhase': 'lfoPhase014',
    'lfoSync': 'lfoSync014', 'lfoContour': 'lfoContour014',
    'accentSweep': 'accentSweep014', 'reverseGate': 'reverseGate014',
}
# Defaults from JC303.cpp's APVTS constructor, not the MPC Init defaults.
UPSTREAM_DEFAULTS = dict(
    waveform=1, cutoff=0, resonance=.92, envmod=0, decay=.29, accent=.78,
    volume=.75, reverseGate=0, normalDecay=.3, accentDecay=.03,
    accentSweep=0, feedbackFilter=.63, softAttack=.26, slideTime=.33,
    sqrDriver=.25, switchModState=0, lfoWaveform='Triangle', lfoRate=.25,
    lfoDepth=0, lfoDestination='Cutoff', lfoPhase=0, lfoSync=0, lfoContour=0,
    modSlot2Dest='Cutoff', modSlot2Amount=0, modSlot3Source='Off',
    modSlot3Dest='Cutoff', modSlot3Amount=0, modSlot4Source='Off',
    modSlot4Dest='Cutoff', modSlot4Amount=0, overdriveModelIndex=1,
    overdriveLevel=.15, overdriveDryWet=.35, switchOverdriveState=0,
    overdriveDisperse=0, overdrivePreVca=0, filterType='Spicy', oscModel='TB-303',
    junoWave='1 Saw', superDetune=0, superMix=.75, shaperMode='Tri Mod',
    shapeDrift=0, shaperSplit=0, squareModel='Circuit', filterDrive=0,
    filterMorph=0, bassComp=0, filterFm=0, resTrim=.5,
    filterQuality='Standard', filterTension=0, filterBias=.5, filterRail=1,
    subOscGain=0, subOscBlend=1,
)


def records(root):
    params = json.loads((root / 'params.json').read_text())['params']
    indexes = {p['key']: i for i, p in enumerate(params)}
    result = []
    files = sorted((root / 'factory/jc303-v014').glob('*.json'),
                   key=lambda p: (p.stem != 'init', p.stem))
    assert len(files) == 18
    for path in files:
        preset = json.loads(path.read_text())
        assert set(preset['values']) <= set(UPSTREAM_DEFAULTS), path
        upstream = UPSTREAM_DEFAULTS | preset['values']
        values = {}
        for key, value in upstream.items():
            target = ALIASES.get(key, key)
            index = indexes[target]
            p = params[index]
            if isinstance(value, str):
                value = p['options'].index(value)
            value = float(value)
            if key == 'slideTime':
                value *= 358 / 998  # retain 2..360 ms in MPC's 2..1000 ms range
            elif key == 'filterDrive':
                value *= 16  # upstream normalized pot -> MPC dB
            elif key == 'resTrim':
                value = .85 + .30 * value
            elif key == 'filterTension':
                value = 0 if abs(value) < .05 else (abs(value)-.05)/.95 * (1 if value > 0 else -1)
            low, high = (0, len(p['options']) - 1) if 'options' in p else (p['min'], p['max'])
            assert low <= value <= high, (path.name, key, target, value)
            values[index] = value
        values[indexes['engine014']] = 1
        for key in ('circuitEnabled', 'oscModsEnabled', 'filterModsEnabled',
                    'modSlot1Enabled', 'modSlot2Enabled', 'modSlot3Enabled', 'modSlot4Enabled'):
            values[indexes[key]] = float(upstream['switchModState'])
        # The old three-target LFO mirror cannot represent the extended targets.
        values[indexes['lfoDestination']] = min(values[indexes['matrixDest1']], 2)
        result.append(dict(name=preset['name'], source=path.name,
                           sha256=hashlib.sha256(path.read_bytes()).hexdigest(),
                           values=sorted(values.items())))
    return result


def generate(root):
    presets = records(root)
    rows = ['// Generated from factory/jc303-v014; GPL-3.0-only.',
            'struct FactoryValue { int param; float value; };']
    for i, p in enumerate(presets):
        rows.append(f'static const FactoryValue factory_{i}[]={{' +
                    ','.join('{%d,%.9gf}' % (j, v) if v != int(v) else '{%d,%d.f}' % (j, v)
                             for j, v in p['values']) + '};')
    rows += ['struct FactorySound { const char* name; const FactoryValue* values; unsigned count; };',
             'static const FactorySound factorySounds[]={']
    rows += ['{' + json.dumps(p['name']) + f',factory_{i},sizeof(factory_{i})/sizeof(FactoryValue)' + '},'
             for i, p in enumerate(presets)]
    rows += ['};', 'static constexpr int factorySoundCount=sizeof(factorySounds)/sizeof(factorySounds[0]);']
    (root / 'build').mkdir(exist_ok=True)
    (root / 'build/factory_sounds.h').write_text('\n'.join(rows) + '\n')
    (root / 'build/factory-presets.json').write_text(json.dumps(presets, indent=2) + '\n')


if __name__ == '__main__':
    generate(Path(__file__).resolve().parents[1])
