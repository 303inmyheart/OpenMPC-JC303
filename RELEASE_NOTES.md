# JC303 MPC 1.0.0 — Gen2 MPC

## What's new

IMPORTANT: Manufacturer changed from midilab to 303inmyheart. Older personal MPC presets may be rejected as "not for this plugin". Keep originals and use migrated copies; updating the plugin does not convert your user presets.

Adds all 18 original JC303 v0.14 factory sounds: 30 native MPC programs in total. Existing parameter identities and the twelve-choice legacy preset automation range remain unchanged.

## Before updating: personal presets

**Back up your presets and projects first.** The standard installer updates the
plugin and removes its previous matching-id vendor folder; it does **not**
convert personal `.xpl` headers. An old preset can still name `midilab` and be
rejected by MPC before the plugin reads its sound state. Keep the original and
create a compatible copy with updated manufacturer and installed-file metadata;
leave the encoded sound payload and UID unchanged.

[Migration instructions](https://github.com/303inmyheart/OpenMPC-JC303/blob/jc303-mpc-v1.0.0/PRESET-MIGRATION.md)

Corrected-copy loading was confirmed on Live III for one historical JC3
preset. Broader historical preset/project acceptance still needs verification.

## Hardware and CPU

Final exact ARM64 binary tested on MPC Live III / MPC OS 3.10.1:

| Benchmark | Worst p99 | Worst block | Verdict |
|---|---:|---:|---|
| Standard OpenMPC | 34.4% | 37.5% | WARN |
| Additional full-parameter stress | 33.9% | 38.2% | WARN |

These are audio-block budget percentages on one A76 core, not the MPC project
CPU meter. WARN is not PASS and does not establish multi-instance capacity.
[Full reports](https://github.com/303inmyheart/OpenMPC-JC303/tree/jc303-mpc-v1.0.0/benchmarks/1.0.0).
Native ASan/UBSan, actual ARM64/QEMU VST checks, 30 preset-state exports and
installer migration checks passed; current catalog validation has zero warnings.
One historical user-preset conversion and loading in the prepared 0.7.4 project
test were confirmed by the maintainer. This does not claim all old projects,
all factory sounds or all menu interactions were exhaustively hardware tested.

## Included tutorial

PRESET-MIGRATION.md and CHANGELOG.md are included inside the installed plugin
folder as well as the source repository. PRESET INFO opens the plain-language
English guide inside the plugin. No converter runs automatically.
