# JC303 MPC and MPC porting roadmap

Status: 2026-11-27. Target: Gen2 MPCs (aarch64), MPC OS 3.10.1+.
Hardware testing has been performed on MPC Live III only.

## Release 1.0.0

- All 18 original v0.14 factory sounds added; 30 native MPC programs and XPLs.
- Manufacturer 303inmyheart; standard installer retires the matching old folder.
  User XPL headers are not converted automatically: see PRESET-MIGRATION.md.
- English PRESET INFO opens the plain-language guide on every page. Tutorial
  and changelog are included in the release ZIP, alongside the plugin.
- One historical preset copy was confirmed to load. The maintainer also
  reported plugin loading in the old-0.7.4-project test. Full saved-setting,
  all-project and all-menu acceptance is not implied by these reports.
- Final exact-binary Live III benchmark: standard WARN 34.4% p99 / 37.5% max;
  extra full stress WARN 33.9% / 38.2%. See benchmarks/1.0.0/README.md.
- Local native/ARM64, skin, preset and installer checks pass; current catalog
  validation has zero warnings. Tutorial and measured installer accompany the release.

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

## Preset migration follow-up

- Keep the confirmed header-copy path and manual tutorial documented. Verify
  native Save-as from an old project as the simpler route for nontechnical users.
- A guided opt-in file converter remains future work. Never overwrite original
  user presets or change their encoded sound data automatically.
- Extend testing across older projects/presets and collect exact error reports.

## Deferred JC303 work

1. Original factory bank is implemented for 1.0.0; user JSON bank import/export
   and management remain deferred. Exhaustive listening to all 18 factory sounds
   and extended project tests remain validation work.
2. Optional sequencer extras: pitch-only/rhythm-only randomization, internal/MIDI
   clock and sequencer MIDI output. External host MIDI capability needs review.
3. Further performance work and extended real-project/multiple-instance testing.
   A changed release binary needs a fresh matching benchmark.

## Deferred issue: Sounds → Instruments thumbnail navigation

- Reported on MPC Live III / MPC OS 3.10.1: after loading a sound, tapping
  the already selected plugin thumbnail does not reopen its preset/kit list.
- The maintainer confirms the same behavior in **Geonkick MPC and JC303 MPC**;
  **Hype works** in the comparison test. The shared cause remains unconfirmed.
- Workaround confirmed for Geonkick: use the arrows beside the current preset
  at the bottom of Sounds, or the native MPC preset selector inside the plugin.
- Deferred at the maintainer's request: the same failure was also confirmed
  with other external plugins, while Hype works. No port-specific fix is claimed.
- This concerns Sounds-browser navigation, not opening the plugin editor.
  It is separate from missing artwork in the OpenMPC Plugin Manager.

## Plugin Manager artwork

- JC303 has no image in the OpenMPC Plugin Manager (VST), reported by the
  maintainer. Investigate the manager/catalog image binding and cache. This is
  separate from the Sounds-browser navigation issue above; do not mark it fixed yet.

## Packaging policy

- Implemented in 1.0.0: explicit format:vst2, current reviewed installer and
  uninstaller templates, release helpers pinned132b0db05c3903f1aeb37a5021a0b556811eb55a.
- Recheck the current contribution rules and validator before future releases.
- Published 0.7.4 assets/tag remain immutable; every new binary needs its own
  matching tests and final benchmark. Future notes must lead with breaking
  preset/project changes, so Plugin Manager's shortened What's new retains them.

## Deliberately out of scope

- Microtuning / AnaMark .tun file import.
- Additional external MIDI/macro controls, pitch-wheel/mod-wheel integration and
  the desktop assignable-mod-knob workflow.

Existing MIDI notes, pad playing, sustain/step recording and native MPC Q-Links
remain. Key/scale snapping is a future decision; current scales are a visual guide.

## Other ports

1. **Geonkick MPC** — 1.0 is published; 1.0.1 manufacturer correction is
   installed for testing. Next: equivalent preset/project migration checks,
   English in-plugin instructions and tutorial/release notes. Curve points
   receive no Q-Links.
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
