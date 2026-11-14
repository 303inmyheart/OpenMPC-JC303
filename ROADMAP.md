# JC303 MPC and MPC porting roadmap

Status: 2026-11-14. Target: Gen2 MPCs (aarch64), MPC OS 3.10.1+.
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

**0.4.2 is currently installed on the maintainer's Live III.** The maintainer
confirmed the corrected ARTIC switches and the SEQUENCE Slide/Accent/Vibrato
touch lanes. This is a tested development build, not a published release or
catalog update. Full extended stability/multiple-instance validation is separate.

Development milestones since 0.2.3:

| Build | Implemented locally | Validation/status |
|---|---|---|
| 0.3.0 | JC303 v0.14.0 synthesis: eight filter models, TB-303 / Alpha Juno / Phaseshaper oscillators, Circuit square, sub oscillator and filter shaping; original engine retained for older projects | Installed and exercised on Live III; native sanitizer and ARM64 checks passed |
| 0.3.1 | ENGINE and FILTER grouped beneath MODS, with matching Q-Link banks | Installed; maintainer confirmed navigation |
| 0.4.0 | 64-step piano roll on four 16-step pages; active-length generation; mute, hammer, reverse, swing, fractional gates, pitch polymeter, clipboard and undo/redo | Installed; superseded after touch/input feedback |
| 0.4.1 | Moving playhead line; 12 keys and 19 scales plus Off; canonical touch feedback during Stop/Play | Installed; pitch improvement confirmed, persistent switch problem identified |
| 0.4.2 | Separate native input recipes for persistent switches and momentary cells; reversible GATE/TIE editing | Installed; maintainer confirmed the touch correction works |
| 0.5.0 | Four-slot matrix with native MODS pages, 8 source choices / 14 destinations, expanded LFO destination; Host Transport / Note Gate-Hold / latched Note Trigger and automatable Q-Link STOP | Installed for testing; offline checks pass, Live III acceptance pending; next release only |

The 64-step editor includes:

- SEQUENCE, ARTIC, SEQTOOLS, SEQLIVE and SEQKEY under one main function key.
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
**No formal/store benchmark has been run for these development builds.** It is
reserved for the completed release build. Matching source and release metadata
must be published together before a new public binary release.

## Local 0.5.0 candidate: matrix and note-start modes

Implemented for the **next public release**, not in the current 0.2.3 download:

- Four slots: fixed LFO, fixed bipolar envelope and two selectable sources.
- Eight source choices and fourteen destinations, including transient DRIVE
  Dry/Wet modulation. Source Off / Amount 0 disables the additional route.
- MATRIX1–4 beneath MODS, with matching Q-Link banks and engine/MODS status.
  Expanded destinations require the JC303 0.14 synthesis engine and MODS on.
- LFO page shares destination/depth with slot 1; old parameter IDs are retained.
- Host Transport stays the default. Note Gate/Hold plays while notes are held;
  Note Trigger latches until Stop. Both use MPC tempo and preserve pattern pitch.
- An automatable STOP impulse is on TOOLS and a free Q-Link. Actual MPC automation
  selection/recording needs hardware acceptance, distinct from VST API checks.
- JC8 projects store all slots and the selected start mode. Old projects retain
  their existing LFO route and host clock; new routes are neutral. Held/latched
  triggers do not resume when loading a project. Older builds cannot read JC8.

**Device acceptance of 0.5.0 is pending.** There is no new public binary release
or release benchmark. See [the test guide](TESTPLAN-0.5.0-DE.md).

## Remaining planned JC303 work

1. LFO extensions: start phase, key sync, contour and remaining modulation routing.
2. Additional MODS controls: Accent Sweep and global Reverse Gate. Per-step
   reverse is already present.
3. Drive extensions: Pre-VCA routing and Disperse. Current post-VCA GuitarML
   models and the optimized inference path are already available.
4. Original factory JSON sounds/banks and practical preset import/export and
   management alongside the existing MPC preset browser.
5. Deferred sequencer extras: pitch-only/rhythm-only randomization, optional
   internal/MIDI clock and sequencer MIDI output. Host clock plus the two
   note-start modes already work in the local candidate.
6. Release validation: extended sessions, multiple instances, parameter-change
   spikes and the final-release Store benchmark, then public source/release and
   catalog update.

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
- Device installation/application restart currently require the maintainer's Go.
- The v0.14 port combines GPLv3 and file-specific AGPLv3 parts; existing MIT/BSD
  notices remain. See LICENSE-AUDIT.md and src/VENDORED.md in the matching source.
