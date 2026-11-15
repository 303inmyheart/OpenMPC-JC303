# JC303 MPC and MPC porting roadmap

Status: 2026-11-15. Target: Gen2 MPCs (aarch64), MPC OS 3.10.1+.
Hardware testing has been performed on MPC Live III only.

## Release 0.7.4

[JC303 MPC 0.7.4](https://github.com/303inmyheart/OpenMPC-JC303/releases/tag/jc303-mpc-v0.7.4)
brings the implemented v0.14 synthesis, sequencer and modulation work into one
public release. Earlier 0.3–0.7 candidates are development history, not separate
required upgrade steps.

- Eight filter models, three oscillator families, Circuit square, sub oscillator
  and extended shaping; legacy synthesis retained for older projects.
- 64 steps on four pages; touch piano roll and articulation; playhead; clipboard,
  undo/redo, rotation, pitch polymeter and generation length 1–64.
- Twelve roots and nineteen scales plus Off as a visual guide.
- Host Transport, Note Gate/Hold and latched Note Trigger; hardware Stop support.
- Four-slot matrix, independent global/page/slot bypasses, extended LFO,
  Accent Sweep and global Reverse Gate.
- Pre/Post-VCA neural Drive and Disperse; six menu groups, knob-focused Q-Links
  with the Drive Model exception, native frames and popup focus.
- New instances and Init: all eight MODS switches Off; Circuit page points to
  MODS MASTER in SETUP. Saved and historical preset/project routing preserved.
- Native/ARM64/UI/installer checks; actual exact-binary Live III benchmarks:
  standard WARN, p99 34.6% / max 38.3% and additional full stress WARN, p99 33.5% / max 43.0%.
  See [measurements](benchmarks/0.7.4/README.md). Multiple-instance and extended
  user sessions remain ongoing validation, not claimed completed here.
- Real Live III captures in the [gallery](docs/GALLERY.md); current SETUP confirms
  Off defaults, older captures document the shared 0.7 interface.

## Deferred JC303 work

1. Original factory JSON sounds/banks and practical preset import/export and
   management. Deferred by the maintainer.
2. Optional sequencer extras: pitch-only/rhythm-only randomization, internal/MIDI
   clock and sequencer MIDI output. External host MIDI capability needs review.
3. Further performance work and extended real-project/multiple-instance testing.
   A changed release binary needs a fresh matching benchmark.

## Deliberately out of scope

- Microtuning / AnaMark .tun file import.
- Additional external MIDI/macro controls, pitch-wheel/mod-wheel integration and
  the desktop assignable-mod-knob workflow.

Existing MIDI notes, pad playing, sustain/step recording and native MPC Q-Links
remain. Key/scale snapping is a future decision; current scales are a visual guide.

## Other ports

1. **Geonkick MPC** — local 0.0.13 prototype installed on Live III; development
   paused while the JC303 update is completed. Three layers/nine oscillators,
   waveform/envelope display and touch point editing exist. Further validation,
   interface refinement and release work remain. Curve points receive no Q-Links.
   Upstream: https://github.com/quamplex/geonkick
2. **Fire** — queued; source/license/dependency and ARM64 feasibility review.
   Upstream: https://github.com/jerryuhoo/Fire
3. **Wolf Shaper** — queued after Fire; feasibility review pending.
   Upstream: https://github.com/wolf-plugins/wolf-shaper
4. **BYOD** — navigation/usability feasibility review before committing to a port:
   adding, connecting and editing blocks must be practical on MPC touch/Q-Links.
   Upstream: https://github.com/Chowdhury-DSP/BYOD
5. **Surge** — queued after BYOD; license, dependency, ARM64 performance and MPC
   navigation feasibility review pending.
   Upstream: https://github.com/surge-synthesizer/surge

## Delivery and licensing

- Product/installed name: JC303 MPC; catalog display name: JC303. Repository URL
  and plugin identity stay unchanged.
- Development installers stay under build/releases; no Desktop copies by default.
- Builds must remain OpenMPC Installer compatible, with scripts, manifests and
  checksums. Native and ARM64 tests precede packaging.
- Development installation and hardware acceptance are separate from a public release.
- The v0.14 port combines GPLv3 and file-specific AGPLv3 parts; existing MIT/BSD
  notices remain. See LICENSE-AUDIT.md and src/VENDORED.md in the matching source.
