# JC303 MPC and MPC porting roadmap

Status: 2026-11-15. Target: Gen2 MPCs (aarch64), MPC OS 3.10.1+.
Hardware testing has been performed on MPC Live III only.

## Published release

**JC303 MPC 0.2.3** is the latest published GitHub release and the catalog/store
version. [Release and installer](https://github.com/303inmyheart/OpenMPC-JC303/releases/tag/jc303-mpc-v0.2.3).

It includes the original synthesis/mods/LFO, 16-step host-synchronised sequencer,
37 GuitarML models, native MPC preset browser with dry Init, Acid artwork,
Q-Link mappings and Sounds-browser tile. The maintainer tested the release on
Live III / OS 3.10.1. Its recorded Store benchmark is WARN; that historical
result does not describe the newer development builds below.

## Working development builds — not publicly released

The maintainer confirmed **0.5.0** works on Live III, including the matrix and
note-start modes. **0.6.0** adds independent MODS bypasses and the remaining
LFO/Circuit controls. Local **0.7.0** adds Drive Pre-VCA/Disperse and navigation/Q-Link restructuring; hardware acceptance is pending. These are development
builds, not a published release or catalog update.

Development milestones since 0.2.3:

| Build | Implemented locally | Validation/status |
|---|---|---|
| 0.3.0 | JC303 v0.14.0 synthesis: eight filter models, TB-303 / Alpha Juno / Phaseshaper oscillators, Circuit square, sub oscillator and filter shaping; original engine retained for older projects | Installed and exercised on Live III; native sanitizer and ARM64 checks passed |
| 0.3.1 | ENGINE and FILTER grouped beneath MODS, with matching Q-Link banks | Installed; maintainer confirmed navigation |
| 0.4.0 | 64-step piano roll on four 16-step pages; active-length generation; mute, hammer, reverse, swing, fractional gates, pitch polymeter, clipboard and undo/redo | Installed; superseded after touch/input feedback |
| 0.4.1 | Moving playhead line; 12 keys and 19 scales plus Off; canonical touch feedback during Stop/Play | Installed; pitch improvement confirmed, persistent switch problem identified |
| 0.4.2 | Separate native input recipes for persistent switches and momentary cells; reversible GATE/TIE editing | Installed; maintainer confirmed the touch correction works |
| 0.5.0 | Four-slot matrix with native MODS pages, 8 source choices / 14 destinations, expanded LFO destination; Host Transport / Note Gate-Hold / latched Note Trigger and automatable Q-Link STOP | Installed; maintainer confirmed it works; next release only |
| 0.6.0 | Independent Circuit/Osc/Filter/four-slot bypass, LFO phase/key sync/contour, Accent Sweep/global Reverse Gate, hardware-Stop MIDI regression | Installed for testing; hardware acceptance pending; next release only |
| 0.7.0 | Pre/Post-VCA routing, Disperse, six logical function keys, knob-only row banks plus Drive model, native-bound focus frames | Installed for testing; hardware acceptance pending; next release only |
| 0.7.3 | Coalesced waveshaper/view updates, constant-time parameter lookup, inactive-filter coefficient work avoided; unchanged sound quality and UI | Installed; standard and extra full-stress WARN; native/ARM64 and exact real-device audio pass; hearing/touch acceptance pending; next release only |
| 0.7.4 | New instances/Init default all eight MODS switches to Off; Circuit page reminder points to SETUP master; saved/factory routing preserved | Installed; native/ARM64/UI/catalog pass; hearing/touch acceptance pending; next release only; matching final release benchmark required |

The 64-step editor includes:

- SEQUENCE, ARTIC, SEQTOOLS, SEQLIVE, SEQKEY and GENERATE under one main function key.
- Direct note/octave entry, selected-step frame and moving playhead line.
- GATE, SLIDE, ACCENT, VIBRATO, TIE, MUTE, HAMMER and REVERSE lanes.
- Length 1–64 on both the editor and GENERATE. Generation affects only the active
  length; later stored steps remain available when the pattern is lengthened.
- Shuffle, step length, Full/Dotted/Triplet scale, reverse playback, 303 timing,
  pitch length, step/pitch rotation, page/pattern clipboard and undo/redo.
- Key/scale row highlighting on all four pages, stored with the pattern. It does
  not transpose existing notes, quantize input or constrain the generator.
- Older project migration: legacy synthesis/timing remain available; JC7 stores
  all 64 steps and the key/scale guide. Older plugin versions cannot read JC7.

Native ASan/UBSan/leak checks, actual ARM64 shared-object tests under QEMU,
independent timing/scale oracles, native-skin checks and OpenMPC Installer
validation pass. These are distinct from the maintainer's real-device tests.
The maintainer explicitly authorized benchmarks of completed 0.7.0 and the
subsequent optimization comparisons. Local 0.7.3 is WARN in both the standard
run (31.5% p99 / 39.0% max) and the additional all-parameter stress
(31.5% / 44.5%); baseline full stress was FAIL (188.5% / 201.7%). See the
[complete comparison and preserved failures](benchmarks/0.7.3/README.md).
These are per-core audio-block measurements, not the project CPU meter.
Matching source and release metadata
must be published together before a new public binary release.

## Local 0.5.0 candidate: matrix and note-start modes

Implemented for the **next public release**, not in the current 0.2.3 download:

- Four slots: fixed LFO, fixed bipolar envelope and two selectable sources.
- Eight source choices and fourteen destinations, including transient DRIVE
  Dry/Wet modulation. Source Off / Amount 0 disables the additional route.
- MATRIX1–4 beneath LFO in 0.7.0 (previously MODS), with source/destination touch controls, amount Q-Links and engine/MODS status.
  Expanded destinations require the JC303 0.14 synthesis engine and MODS on.
- LFO page shares destination/depth with slot 1; old parameter IDs are retained.
- Host Transport stays the default. Note Gate/Hold plays while notes are held;
  Note Trigger latches until Stop. Both use MPC tempo and preserve pattern pitch.
- An automatable STOP impulse is on TOOLS and a free Q-Link. Actual MPC automation
  selection/recording needs hardware acceptance, distinct from VST API checks.
- JC8 projects store all slots and the selected start mode. Old projects retain
  their existing LFO route and host clock; new routes are neutral. Held/latched
  triggers do not resume when loading a project. Older builds cannot read JC8.

**The maintainer confirmed 0.5.0 works.** There is no new public binary release
or release benchmark. See [the test guide](TESTPLAN-0.5.0-DE.md).

## Local 0.6.0 candidate: independent MODS and phase 2

- MODS MASTER remains global; Circuit, Oscillator, Filter and each matrix slot
  have separate saved bypasses with Q-Links. LFO shares the slot-1 switch.
- Start phase, key sync and bipolar contour use the pinned v0.14 LFO.
- Accent Sweep (15–2000 ms) and global Reverse Gate are on the Circuit page.
- Hardware Stop/All Notes Off clears a latched note trigger even when host
  transport is already stopped. Mode selection remains; a new note can restart.
  This MIDI path is tested offline; actual hardware acceptance remains pending.
- JC9 stores switches and new settings. JC8 defaults to enabled pages and neutral
  new effects; bypass retains knob values. Earlier versions cannot read JC9.

See [the 0.6.0 test guide](TESTPLAN-0.6.0-DE.md). Not publicly released.

## Local 0.7.0 candidate: Drive and navigation

- Original Pre-VCA insert location for the v0.14 engine; Post VCA remains the
  default and the legacy-engine fallback. A short fade avoids abrupt route swaps;
  no double neural inference is used. Routing status distinguishes the fallback.
- Disperse: original eight-stage allpass, Q=1, two-octave spread, 150–3000 Hz
  corner with logarithmic smoothing. Zero and disabled Drive retain dry bypass.
- Six main keys: SYNTH, MODS (Circuit/OSC/Filter/Legacy), LFO (Matrix1–4),
  SEQUENCE (including Generate), DRIVE and global SETUP.
- Q-Links only for visible knobs, with the explicitly requested Drive-model
  exception. Switches/buttons/other selectors remain touch controls. RowTopDown
  banks follow visible knob groups; native label/value bounds define each frame.
- Existing parameter IDs and published MIDI CC mappings remain unchanged. JC10
  stores the two Drive additions; old projects use Post VCA / Disperse Off.

See [the 0.7.0 test guide](TESTPLAN-0.7.0-DE.md). Not publicly released.

## Local 0.7.3: CPU optimization

Square-table requests and derived sequencer feedback are coalesced; parameter
lookup uses an immutable table; inactive TeeBee/Diode coefficient rebuilds are
skipped while both endpoints of model switches remain current. Sound quality,
parameter IDs, saved state, undo depth and the 0.7.0 interface are preserved.
See [measurements](benchmarks/0.7.3/README.md). Not publicly released.

## Remaining planned JC303 work

1. Original factory JSON sounds/banks and practical preset import/export and
   management. Deferred; not part of the current implementation request.
2. Deferred sequencer extras: pitch-only/rhythm-only randomization, optional
   internal/MIDI clock and sequencer MIDI output. Additional host MIDI capability
   needs checking; host clock and both note-start modes are implemented.
3. Hardware acceptance of the new routing/navigation, extended sessions,
   multiple instances and parameter-change spikes. The final-release Store
   measurements for 0.7.3 are recorded; any changed release binary needs its own
   matching benchmark, followed by public source/release and catalog update.

## Deliberately out of scope

The maintainer removed these from future plans:

- Microtuning / AnaMark .tun file import.
- Additional MIDI/macro controls: external pitch-wheel/mod-wheel integration and
  the original desktop's assignable mod-knob workflow.

Existing MIDI note input, pad playing, sustain/step recording and native MPC
Q-Link control remain. Optional key/scale snapping is a separate future decision;
the current key/scale feature is a visual guide.

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
