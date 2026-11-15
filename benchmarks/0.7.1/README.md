# JC303 MPC 0.7.1 – intermediate optimization measurement

First optimization candidate; not installed or published. Same unchanged
OpenMPC host/stages/thresholds as baseline0.7.0: 10 seconds/stage, 44.1 kHz,
128 frames, Live III A76 core4 / 2.4GHz, nice19. MPC active with zero VST2
plugin mappings after reconnect. Private /tmp copies cleaned.

| Run | 0.7.0 p99 / max | 0.7.1 p99 / max | Verdict |
|---|---|---|---|
| All-parameter stress | 188.5% / 201.7% | 48.3% / 62.1% | FAIL remains |
| Four-parameter standard | 34.5% / 57.4% | 33.7% / 40.8% | WARN remains |

Full-stress mean falls from160.4% to28.5%; normal note stages stay around4%.
This measures setter work, not a claim of faster fixed-parameter synthesis.

Changes: retain the final Square Drive table request until rendering; generate
only the active voice's table when its effective drive changes. Derived
sequencer view differences are collected by the existing idle notification
worker, while canonical getters remain immediate. No DSP equations, quality,
sample rate, automation IDs, saved controls or undo depth changed.

Native sanitizer, full ARM64/QEMU and callback/input regressions pass. Native
and actual-device old/new comparisons each cover393216 bit-identical samples
under drive/bypass/step/scale bursts. Device comparison text is retained here.

Candidate stripped SHA-256:
`74a4fcbd4bbc0f2675b15334352ab508e811feab1a0246abcf4bf56086585615`.
0.7.0 remains installed while the next lookup optimization is tested. All FAIL
and WARN reports are retained; nothing was repackaged or published to hide them.
