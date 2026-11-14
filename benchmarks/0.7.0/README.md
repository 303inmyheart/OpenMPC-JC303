# JC303 MPC 0.7.0 – Live III benchmark

Measured on real Live III, OS 3.10.1 (maintainer reported), 2026-11-15 CET.
The maintainer opened a new empty project; MPC stayed running. Private plugin
and benchmark copies ran only in /tmp and were removed. No installed plugin,
project, global settings or service was changed by the measurement.

Frozen installed/installer binary SHA-256:
`e2beacd52606c00c00838ae703e169e6c6a1fe3a208764850b05ecf48bea8d6e`.
OpenMPC tool ref `a2dd2b78a19c4cd46257c09eb9a485a155ab2968`. Core 4 / Cortex-A76, performance
governor, 2.4 GHz, SCHED_OTHER nice 19. 44.1 kHz / 128 samples (2902 us budget).
Percentages are fractions of one core/block budget, not the MPC GUI CPU meter.

## Official unmodified OpenMPC stages

| Run | Settings | Worst p99 | Worst block | Verdict |
|---|---|---:|---:|---|
| Default parameter sweep | 4 random non-trigger parameters per block | 34.5% | 57.4% | WARN |
| Additional full-parameter stress | all 348 non-trigger parameters per block (`-p`) | 188.5% | 201.7% | FAIL |

Both used 10 seconds per stage and 1/4/8/16 note chords. The synth is monophonic;
those chord counts are not independent polyphonic voices. All 270 one-shot
parameters are excluded from parameter sweeps and exercised separately. Default
note-only stages are around 3.9–4.0% mean. The full sweep result is retained, not
replaced by the less demanding default result. `bench.txt`/`bench.json` contain
the full sweep; `bench-standard.*` contains the default run.

WARN is supported by OpenMPC with performance disclosure; the failed additional
stress case remains an optimization/release concern. No release was published.

## Supplemental fixed configurations

Same exact binary. Not the official verdict and not a touchscreen/whole-project
test. Each scenario has three 12-second audio-equivalent repeats, a 16-step Acid
Steps pattern on the new sequencer, 120 BPM, modern core and MODS on. Blackstar
is model 16 (BlackstarHT40_GainKnob_SM57mic), Level 35%, Mix 35%. Spicy HQ adds
filter model 1 and quality on.

| Scenario | Mean (3 runs) | Highest p99 | Highest block |
|---|---:|---:|---:|
| modern+sequence dry | 4.01% | 4.21% | 4.54% |
| Blackstar post | 9.37% | 9.62% | 9.81% |
| Blackstar pre | 9.44% | 9.69% | 10.01% |
| Blackstar post+disperse | 9.56% | 9.80% | 10.00% |
| Blackstar pre+disperse | 9.60% | 9.84% | 10.13% |
| Spicy HQ dry | 9.82% | 10.06% | 10.29% |
| Spicy HQ+Blackstar pre+disperse | 15.30% | 15.58% | 15.82% |

The nonlinear Drive path adds roughly 5.4 percentage points to the dry reference;
Disperse adds about 0.2 points here. Spicy HQ plus Pre-VCA/Disperse is around
15.3% mean in this fixed configuration. Values depend on settings and do not
cover every model/filter combination, GUI interaction, simultaneous instances
or a complete MPC project. The helper/timer and fixture are recorded in
`tools/config_bench.c`; raw repeats are in `configurations.json`.

The audio thread uses CLOCK_THREAD_CPUTIME_ID; process-worker percentages are
reported separately by the stock test and have the tool's documented limits.
No affinity/governor/global settings were changed on MPC processes: the helper
sets only its own affinity/priority. This report is the pre-optimization baseline.
