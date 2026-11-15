# JC303 MPC performance

Most recent measured candidate **0.7.3**: standard OpenMPC run **WARN**, p99 **31.5%**,
maximum **39.0%**. Additional full 348-parameter stress is also **WARN**,
p99 **31.5%**, maximum **44.5%**; 0.7.0 was FAIL at **188.5% / 201.7%**.
The sweep stage itself improves from 160.4% mean to 13.4% mean. These are
single-core audio-block measurements on the actual Live III, not project-meter
percentages. No quality reduction. See [complete reports](benchmarks/0.7.3/README.md),
including all intermediate failures and exact binary hashes. This candidate is
not yet publicly released; historical public 0.2.3 remains WARN at 16.5% / 24.6%.
The subsequent 0.7.4 defaults/UI update has a different binary and needs a new
matching benchmark before public release; do not reuse the 0.7.3 CPU metadata.

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
