# Vendored sources

JC303 upstream: https://github.com/midilab/jc303
commit `25d0ed1dab6d0edffc0e28c7d68c170a8af71aee`.
`src/dsp/open303/` is copied from that commit. The upstream directory's MIT
license is retained. Adapter parameter mappings follow JC303's GPLv3 processor;
the full port is licensed GPL-3.0-only.

Local engine changes:

- `rosic_MipMappedWaveTable.cpp`: clear exactly `tableLength` prototype entries,
  not `tableLength+4`. The prototype is `double[2048]`; interpolation padding
  belongs to the mipmapped tables, not this array. UBSan caught the overrun.
- `GlobalDefinitions.h`: use `std::uint32_t` for UINT32 on LP64; replace
  pointer type-punning in exponent extraction with C++20 `std::bit_cast` and
  fixed-width integers. This avoids undefined aliasing and 64-bit reads of floats.

No filter or oscillator algorithm changes. Mod-disabled defaults are explicitly
restored; the adapter avoids upstream's constructor-order inconsistency where
some mod parameters were re-applied after disabling modifications.

Sequencer milestone (2026-11-13): `src/sequencer/AcidSequencer303.h` and
`Harmonizer.h` are also copied from the same JC303 commit. The retained headers
attribute the Aciduino port to Romulo Silva (2022), under MIT. `LICENSE` records
that notice. Local changes remove JUCE buffer/lock dependencies in favour of
the wrapper's engine lock and expose `tick96()`. The desktop MIDI-buffer
entrypoint is removed. The MPC-specific `HostClock.h` supplies tick timing from
the host position, tempo and cycle; the retained desktop clock helpers are not
called. Existing gates are decremented before creating new ones, fixing the
upstream 12-tick half-step gate being shortened to 11 ticks. Pattern generation,
note selection, accents and slide/tie lookahead remain the vendored algorithms.

`MpcSequence.h` connects those events directly to the voice and adds parameter
editing, recording, transactional pattern state and host notifications. Counts
per MIDI pitch prevent one overlapping note-off from releasing another held
sequence note of the same pitch. No subprocess or external MIDI bridge is used.

`vendor/math_approx/include/` is from
https://github.com/Chowdhury-DSP/math_approx at
`0c68d4d17242d707ba07fa7f1901692b7ed72d58`, matching JC303's dependency pin.
No changes. BSD-3-Clause license retained.

MPC wrapper/tools: https://github.com/open-mpc/mpc-vst-plugins at
`ef05a26c6e59f1bfd3ef2023e6a9d8b371b61310`, with
`patches/mpc-float32.patch` applied in the neighbouring checkout (MIT).

`art/DejaVuSans.ttf` is the host's DejaVu Sans font, used to rasterise labels
at build time. `art/FONT-LICENSE.txt` retains its notices. No font is needed on
the device beyond MPC's built-in live label fonts.

## GuitarML milestone (0.2.0)

The 37 JSON models and original model order/names come from the pinned JC303
checkout, src/dsp/guitarml-byod/models/JC303 and GuitarMLAmp.h. Their bytes are
embedded by tools/embed_models.py; model filenames identify saved states.
RTNeural is pinned at 04cb333bc4b174760958a77c7ce076eae38fe8e4 (BSD-3-Clause),
with xsimd a00c81f7b9e808a42aedcf7da2fbb1f9a636da34 (BSD-3-Clause) and the
RTNeural-bundled nlohmann JSON header (MIT). The RNN adapter/model loader derive
from JC303/ChowDSP (GPL-3.0; CHOWDSP-LICENSE.md), original ChowDSP revision
ffc70ba399f9afaeefb996eb14e55a1d487270b8.

Local fixes: RTNeural lstm_xsimd.h kernel_mat_mul bounds the last SIMD input
batch by in_size. Two-input models otherwise read nonexistent kernel rows 2/3,
detected by UBSan. The independent scalar LSTM oracle exercises this path.
Unused recurrent shapes were removed from the adapter; model_loaders returns
the local value directly. Inference uses the original fast activation functions,
44.1 kHz model rate, residual output and DC highpass. Model loading/warmup and
switch scheduling are implemented locally in Drive.cpp.

0.2.1 optimization: recurrent_mat_mul stores input lanes once and accumulates
in a local SIMD array, unrolling the small output loop. Scalar addition order
is preserved. The fixed 44.1 kHz adapter uses SampleRateCorrectionMode::None,
with reset on preparation; the original LinInterp(1) instantiations remain for
the all-model parity test. No model weights or activation approximations change.

The model-overlay skin uses documented native MPC UI API primitives (Image,
Button, CustomComponentDefinition and KnobOverlay), referenced from the official
MPC SDK ui-reference.md. No stock Akai artwork, firmware or UI JSON files are
distributed; its panel/choice artwork is generated locally from this project's code.

## v0.14 synthesis milestone (0.3.0)

`src/dsp/v014/` is from midilab/jc303 tag v0.14.0, commit
`b0ff26011f80e45c7796cad7b29fbc7f347b9337`. Source licenses and copyright
notices are retained. Namespace tokens rosic/dfl become rosic014/dfl014 to allow
coexistence with the untouched old synthesis engine. ModernVoice uses a private
implementation so differently versioned headers do not share include guards.
The global numerical helpers remain equivalent; the existing LP64/bit_cast
GlobalDefinitions safety fix is reused. The existing prototype-table bound fix
is also retained. Ooura FFT implementation symbols are confined to the modern
translation unit's anonymous namespace to avoid duplicate global functions.
GlobalFunctions.cpp is not compiled twice. Object filenames receive a v014_
prefix; legacy compilation/source filenames are unchanged.

New engine classes include all upstream oscillator/filter algorithms, but this
milestone exposes synthesis only: oscillator models, square model, Juno spread,
phaseshaper, sub oscillator, eight filters and shaping/quality controls. The old
three-target LFO remains mapped. New mod-matrix controls, v0.14 sequencer events,
pre-VCA drive/Disperse, factory JSON presets and .tun import are later milestones.
The tuning source is retained as a dependency of the modern voice, not a public
file-import feature. Our optimized post-VCA GuitarML inference is unchanged.

`MpcSequence` retains its old timing and adds a routed note/off callback, directing
its events to the selected synthesis engine. It never runs both audio cores.
JC303-MPC-1/2/3/4 state selects the legacy engine, preserving old filter indices
including Diode Octave. JC303-MPC-5 adds the engine choice and 19 sound controls.
All old VST indices, including the implicit popup flag at 65, are retained.

Licensing is now a combined GPLv3/AGPLv3 distribution: see LICENSE-AUDIT.md and
src/dsp/v014/open303/LICENSE-AGPL-3.0. The old MIT/GPL/BSD notices stay in place;
no file is silently relicensed by the directory layout or namespace change.

## 64-step sequencer milestone (0.4.0)

`src/sequencer/v014/AcidSequencer303.h` is adapted from the same pinned upstream
v0.14.0 source (original Aciduino MIT notice retained). JUCE types/clock wrappers
are replaced by standard C++ and the existing MPC HostClock. The host serializes
all calls under its per-instance lock. Native upstream event-queue, swing,
reverse/hammer/mute, pitch-length and step-history logic is retained. MPC changes:
64 steps, active-length-only generation, fractional gate percentage, live flag
overrides, an added step vibrato flag and namespace jc303seq014. The old event
engine/adapter remains as LegacyMpcSequence for chunks through version 5.

MpcSequence provides the host bindings, page aliases, grid actions, clipboard,
new JC303-MPC-6 extension and independent alternating/smoothed pitch vibrato.
No D16 code or artwork is used. The local, user-supplied reference image remains
untracked and is not included in source or install archives.


0.4.1/0.4.2 UI feedback: patches/mpc-async-revision-cache.patch applies after
the float patch on the documented wrapper pin. It includes opt-in async idle
engine-change delivery, canonical momentary reset without audio blocks, atomic
pending flags and the UI-thread revision snapshot cache. Its output against
the pinned base plus float patch was byte-compared to the tested wrapper/header.
The core API/header is unchanged. Other ports do not opt into ASYNC_IDLE_NOTIFY
unless explicitly configured. Persistent switches use the existing native MODS
recipe; note/octave actions use the native momentary trigger recipe.


## 0.6.0 independent bypass adaptation

The pinned v0.14 DSP gains one MPC-specific slot-enable array and checked setter
in rosic_Open303.h. Disabled slots are skipped before their source is sampled,
so another slot cannot re-enable a bypassed LFO route by modulating its depth.
Original matrix arithmetic and license notices are unchanged. Phase, key sync,
contour, Accent Sweep and global Reverse Gate call the existing upstream setters.
Independent Circuit/Osc/Filter defaults and saved switches live in the adapter.


## 0.7.0 Drive extensions

DriveDisperser adapts the pinned upstream src/JC303.h Disperser (GPL-3.0-only),
using the already vendored dfl014 DispersionFilter / AllPass2p with their original
file-specific notices. Eight stages, Q=1, two-octave spread, 150–3000 Hz corner
and logarithmic smoothing are retained. The sixteen-sample update cadence
continues across host/MIDI block splits. No new external dependency is added.

The v0.14 Open303 header adds an MPC-only inlined single-sample insert adapter
using its existing renderPreVca/applyVca helpers. This keeps sequencer/MIDI
sample timing and the original insertion location without allocations or
double neural processing. Existing sample()/post-VCA path is unchanged by default.
