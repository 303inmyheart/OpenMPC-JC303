# MPC Live III porting queue

Target: Gen2 MPCs, aarch64, MPC OS 3.10.1+. Hardware tested: MPC Live III only.
Every deliverable goes to the desktop as an OpenMPC Installer compatible ZIP.

1. **JC303 MPC** — released; remaining feature-parity work deferred.
   - Synth voice, filter models, mods and LFO: implemented, user testing ongoing.
   - Stabilise rotary/selection synchronisation; extend slide time to 1 second.
   - 0.1.0: host-synchronised 16-step sequencer, pattern editing, accents,
     slides/ties, generator, MIDI step recording, and project save/restore are
     implemented. The user confirmed the 0.1.0 build works on Live III.
   - 0.2.0: 37 GuitarML models, DRIVE page, native MPC program bank and dry Init.
   - User confirmed 0.2.0 works, including the preset browser (2026-11-13).
   - Deferred by the user for now (resume only as a later milestone): desktop preset/custom model import,
     sequencer MIDI output, internal/MIDI clock modes and note-trigger start.
     The desktop keyboard is replaced by MPC pads and the NOTE control.
   - 0.2.1: Acid / 303 design on all six pages, custom knobs, switches, selectors
     and model chooser; Acid smiley on the first SYNTH page.
   - 0.2.1: measured directly on Live III; Blackstar + sequencer CPU per core
     reduced from 17.76% to 6.78%, with bit-identical captured audio. TS9/RAT
     show similar savings. Device host/voice tests pass; see PERFORMANCE.md.
   - 0.2.2: complete Q-Link control bounds, border-only step selection and
     Sounds-browser banner with a real Init .xpl. Confirmed by the maintainer
     on Live III / OS 3.10.1, including the Sounds tile.
   - 0.2.3: Gen2 scope, neutral drive-model labels, current catalog packaging,
     official CPU benchmark (WARN, p99 16.5%, max 24.6%), catalog entry and published release. Candidate 5 was confirmed on Live III; catalog PR #845 is merged.
   - Remaining validation: multiple instances, parameter
     sweep spikes and extended stability. Feature-parity work stays deferred.
2. **Geonkick MPC** — active next port after JC303 MPC.
   - Upstream: https://github.com/quamplex/geonkick
   - Start with source/build/license/dependency review and ARM64 feasibility.
   - 0.0.1 local single-instrument prototype exists: three layers, nine oscillators, native pages and OpenMPC Installer ZIP. Native sanitizer and ARM64 QEMU tests pass; hardware tests are pending.
3. **Fire** — queued after Geonkick from the user's follow-up link.
   - Upstream: https://github.com/jerryuhoo/Fire
   - Source/build/license/dependency review and ARM64 feasibility are pending.
4. **Wolf Shaper** — queued after Fire.
   - Upstream: https://github.com/wolf-plugins/wolf-shaper
   - Source/build/license/dependency review and ARM64 feasibility are pending.
5. **BYOD** — feasibility review before deciding to implement a port.
   - Upstream: https://github.com/Chowdhury-DSP/BYOD
   - Explicit user requirement: when this target comes up, first assess whether
     navigation and editing effect chains make sense on MPC touchscreen/Q-Links.
     Evaluate adding/removing/reordering/connecting blocks, parameter access,
     browsing and presets. Recommend implementation only if the interaction is
     practical; do not equate successful audio compilation with a useful port.
6. **Surge** — queued after BYOD.
   - Upstream: https://github.com/surge-synthesizer/surge
   - Source/build/license/dependency review, ARM64 performance and MPC navigation
     feasibility are pending. No implementation has started.
