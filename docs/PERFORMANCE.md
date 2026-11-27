# Performance evidence

Current release: [JC303 MPC 1.0.0 exact-binary results](../benchmarks/1.0.0/README.md).
Standard WARN 34.4% p99 / 37.5% max; extra full stress WARN 33.9% / 38.2%.
These are audio-block budget measurements on one A76 core, not the project meter.

## Historical comparison

- [0.7.4](../benchmarks/0.7.4/README.md): standard WARN 34.6% / 38.3%;
  full stress WARN 33.5% / 43.0%, each tied to its exact old binary.
- [0.7.3 optimization](../benchmarks/0.7.3/README.md): native and actual-device
  comparisons each covered 917504 bit-identical samples. DSP quality retained.
- [0.7.0 baseline](../benchmarks/0.7.0/README.md): full stress FAIL 188.5% / 201.7%.
  Intermediate failures and raw reports remain unchanged in the benchmark tree.
- [Early model optimization](../benchmarks/live-iii-0.2.1.json): isolated neural
  inference improvements; not a measurement of the current full plugin.

Historical development write-ups remain accessible in Git history. Do not reuse
an older report as evidence for a different binary or infer multi-instance
capacity from a single audio-thread result.
