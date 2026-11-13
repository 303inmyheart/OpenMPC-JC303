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
