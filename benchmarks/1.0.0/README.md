# JC303 MPC 1.0.0 — final binary, Live III

Both runs measure the installed stripped ARM64 library with SHA-256
`ca9761e252390f79e21daff2765f39dcdc95adb520aeff2edc472b32b89e38c8`.
The maintainer confirmed a new empty project. Zero VST2 plugin mappings were
verified before testing; MPC stayed running. No projects/settings/services
were changed for the benchmark. Private temporary copies were removed.

OpenMPC tools pinned to `132b0db05c3903f1aeb37a5021a0b556811eb55a`.
The official bench.c SHA-256 is
`6a839caaf0482f401c32838123adf1405b248f096d4d210179d7b47df693bca5`,
identical to the host used for 0.7.4. The wrapper only selects core 4 and nice 19;
stages, timing, random sweeps and verdict thresholds are unchanged.
Live III / MPC OS 3.10.1 (maintainer reported), Cortex-A76 core 4, performance
2.4 GHz, 44.1 kHz, 128 samples, ten audio seconds per stage.

| Run | Worst p99 | Worst block | Verdict |
|---|---:|---:|---|
| Standard four-parameter OpenMPC check | 34.4% | 37.5% | WARN |
| Additional full 348-parameter stress | 33.9% | 38.2% | WARN |

Standard background threads reach 3.8% of one core; the full stress run reaches
15.2%. Each run also separately fires 270 one-shot controls (maximum 31.6% and
34.6% of a block respectively). These stages are retained in the raw reports.
The full-sweep stage alone is mean 13.6%, p99 26.2%, maximum 38.2%; its overall
p99 comes from the release-tail stage. Untouched/default held-note stages are
around 1.9–2.0% p99, not a prediction for arbitrary Drive/MODS settings.

The standard result is attached to the release installer. WARN is not PASS:
p99 must be <=15% and maximum <=50% for PASS; WARN allows p99 <=35% and maximum
<=80%. Percentages describe one audio thread's block budget, not the MPC
project CPU meter. This is not a benchmark of every factory sound, every neural
model or multi-instance projects. Preserve original reports; no reruns were
selected to hide failures. Historical 0.7.4 reports remain unchanged.
