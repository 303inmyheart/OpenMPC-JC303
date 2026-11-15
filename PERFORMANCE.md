# JC303 MPC performance

Release **0.7.4**, exact final ARM64 binary on MPC Live III / OS 3.10.1:

| Test | p99 | Max | Verdict |
|---|---:|---:|---|
| Standard OpenMPC | 34.6% | 38.3% | WARN |
| Additional full-parameter stress | 33.5% | 43.0% | WARN |

The installer records the standard result. See [exact final-release evidence](benchmarks/0.7.4/README.md).
These are single-core audio-block measurements, not the MPC project meter.
Baseline 0.7.0 full stress was FAIL 188.5%/201.7%; its unchanged reports and all
intermediate failures remain available. [0.7.3 optimization details](benchmarks/0.7.3/README.md)
explain the changes without reducing audio quality; tested native and actual-device
comparisons each covered 917504 bit-identical samples. The final 0.7.4 adds MODS-Off
Init defaults and a printed SETUP reminder. No 0.7.3 report is reused for0.7.4.
Historical public 0.2.3 was WARN 16.5%/24.6%; it is a different feature set and binary.

## Historical JC303 0.2.1 — Live III performance comparison

Measured on the actual MPC Live III using private copies under `/tmp`, with MPC
still running. The temporary files were removed; no plugin was installed and
no service was restarted. This measures audio-thread time per 128-sample block
at 44.1 kHz on one Cortex-A76 core (core 4, 2.4 GHz), using
`CLOCK_THREAD_CPUTIME_ID`. The helper runs at nice 19. Three runs per scenario,
each processing 12 seconds of audio. These percentages are **not the MPC UI CPU meter**.

| Scenario | 0.2.0 mean / core | 0.2.1 mean / core | Saving |
|---|---:|---:|---:|
| Blackstar HT40 SM57 + sequence | 17.76% | 6.78% | 61.8% |
| Blackstar HT40 SM57 stopped | 16.44% | 5.45% | 66.8% |
| TS9 + sequence | 17.77% | 6.81% | 61.7% |
| RAT + sequence | 17.43% | 6.68% | 61.7% |

Blackstar uses the Acid Steps factory pattern, Level 35%, Mix 35%. The stopped
case leaves the drive enabled and the sequencer/host transport stopped.
The TS9 and RAT cases use the same pattern/level/mix. The full time series
summary and binary SHA-256 hashes are in `benchmarks/live-iii-0.2.1.json`.

262,144 captured Blackstar samples are bit-identical before/after; a repeated
baseline capture is also identical. Native sanitizers, actual ARM64 shared-object
tests under QEMU, and the same host/voice suites on the real Live III pass.
All 37 fixed-rate models match the original LinInterp(1) path sample for sample.

Changes: keep recurrent matrix accumulators local and unroll the small SIMD
loop; compile inference with -O3 and tune ARM scheduling for Cortex-A76;
remove interpolation delay maintenance at the fixed model rate of 44.1 kHz;
cache equal-power mix coefficients when the mix is stationary. No downsampling,
reduced model size, silence gate or fast-math is used.

This does not measure touchscreen drawing, an entire project, or parameter-sweep
worst cases. The new skin still needs testing in the actual MPC UI.
The user reported 30–33% on the MPC meter with the old Blackstar build; do not
substitute these isolated per-core figures for that reading.
