# JC303 MPC 0.7.3 – real Live III optimization comparison

Measured on MPC Live III, OS 3.10.1 (maintainer reported), on the exact final
stripped binary below. Unchanged pinned OpenMPC benchmark host/stages/limits:
10 seconds/stage, 44.1 kHz, 128 frames, A76 core 4, performance governor at
2.4 GHz, SCHED_OTHER / nice 19. MPC application kept running, zero VST2 plugin
mappings checked. Temporary benchmark copies cleaned afterwards.

| Official run | Baseline 0.7.0 p99 / max | 0.7.3 p99 / max | Overall verdict |
|---|---|---|---|
| Four-parameter standard | 34.5% / 57.4% | 31.5% / 39.0% | WARN |
| Additional full 348-parameter stress | 188.5% / 201.7% | 31.5% / 44.5% | WARN (was FAIL) |

In the full sweep stage alone, mean improves from 160.4% to 13.4%, p99 from
188.5% to 26.4%, and max from 201.7% to 44.5%. Overall p99 is 31.5% because
the release-tail stage is the slower remaining stage. The trigger-only stage
covers 270 one-shot actions separately. Both complete runs and all intermediate
failures remain available; no threshold, stage or timing was changed.

Changes accumulated since 0.7.0:

- Apply only the final requested square waveshaper table to the active engine
  at render time; avoid repeated FFT/table rebuilds within a burst of setters.
- Collect derived sequencer-view differences in the existing idle notification
  worker. Canonical getters still return the current pattern immediately.
- Replace a linear 618-key search with an immutable allocation-free lookup,
  initialized before real-time processing.
- Avoid unused TeeBee/Diode cutoff coefficient recalculation; keep both the
  sounding and incoming filter current through model-switch fades. The cached
  cutoff and its coefficients remain paired.

No audio-quality, sample-rate, oversampling, neural-model, parameter-ID, state,
UI-layout or undo-depth reduction. These percentages measure audio-thread CPU
against one block's budget on one core, not the whole-project MPC CPU meter.
Fixed-configuration playback is a separate workload: baseline dry playback was
about 4%, and these changes primarily reduce parameter-edit overhead.

The standard run is the catalog-style check; the all-parameter stress is extra
validation. WARN is not PASS and is not a guarantee against every possible
project overload. Matching source, screenshots and a public release still need
the separate release workflow; this measurement does not publish anything.

Stripped candidate SHA-256:
`1354f721127c8c945f450958a539a7852e3d27002717ae8b476fc000feb34de9`.

Raw text and JSON report pairs are unchanged measurements. See 0.7.0, 0.7.1 and
0.7.2 folders for the baseline and intermediate FAIL/WARN results.

Native and actual-device old/new audio comparisons each cover **917504 samples,
bit-identical** to 0.7.0: legacy/modern engines, drive/bypass/step/scale setter
bursts, all eight filters, moving cutoff/bias, Hi-Fi/FM/saturation and live model
changes. The final filter cases also include note-off/retrigger segments.
See [actual-device comparison output](audio-compare-device.txt). This finite
comparison supports equivalence for those cases; it is not a substitute for
maintainer listening/touch acceptance.
