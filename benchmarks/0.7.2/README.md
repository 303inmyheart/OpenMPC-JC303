# JC303 MPC 0.7.2 – parameter lookup optimization

Local intermediate candidate, not installed or published. Unchanged official
host, stages and thresholds, same settings as 0.7.0 and 0.7.1.

Immutable allocation-free parameter lookup replaces the linear 618-key scan.
No parameter IDs, saved state, sound quality or DSP equations changed.
Native sanitizer, exact ARM64/QEMU and UI checks pass; native and actual-device
comparisons each cover 393216 bit-identical samples against 0.7.0.

| Run | p99 | max | Verdict |
|---|---|---|---|
| Full 348-parameter stress, overall | 35.3% | 46.0% | FAIL |
| Four-parameter standard, overall | 33.6% | 39.0% | WARN |

The full sweep stage improves to mean 15.0%, p99 28.9%, max 46.0%; the release
tail has p99 35.3%, so the overall result remains FAIL. Reports retain this
failure. Baseline full sweep p99 was 188.5%. Timing and limits are unchanged.

Stripped candidate SHA-256:
`69b6c38dce9b17f120cddfa325e9f676e485483d23df00798ad4e6e27df6b4a5`.
0.7.0 remains installed while further optimization is evaluated.
