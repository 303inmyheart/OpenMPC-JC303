# JC303 MPC 1.0.0 validation

Target: Gen2 MPC / aarch64, MPC OS 3.10.1+. Real hardware evidence is from
MPC Live III only. QEMU checks are not hardware benchmarks.

## Automated checks

- Native ASan/UBSan and actual ARM64 shared-object tests under QEMU passed.
- Synthesis/reference audio, legacy state migration, MIDI/transport, 64-step
  editing, scales, modulation matrix, independent bypasses, Drive and Disperse.
- 18 original factory programs: rendering, names, mapping fixtures, unchanged
  historical preset automation, sequence/tuning preservation, edited recall
  and host feedback. All 30 native XPL exports verify complete state roundtrips.
- Skin geometry/bindings and unchanged Q-Link maps after adding PRESET INFO.
- Current catalog check: zero warnings. Isolated installer checks cover old
  vendor retirement, repeated installs, unrelated registry preservation,
  foreign-id protection, native preset paths and uninstall.
- ZIP checksums and privacy audit passed; tutorial/changelog bundled.

See [build instructions](../BUILDING.md) and the [automated tests](../tests/).

## Hardware scope

The maintainer confirmed loading a compatible copy of a historical JC3 preset,
with original file and encoded sound data retained. Loading JC303 in the
prepared 0.7.4-to-1.0.0 project test was also reported. Those checks do not
establish that every old project, setting, factory sound or UI interaction has
been exhaustively tested. Native Save-as from an old project remains a suggested
route to verify, rather than a claimed completed hardware test.

## Exact final binary benchmark

ARM64 SHA256: `ca9761e252390f79e21daff2765f39dcdc95adb520aeff2edc472b32b89e38c8`.
The maintainer confirmed an empty project; zero loaded VST2 mappings verified.

| Run | p99 | Maximum | Verdict |
|---|---:|---:|---|
| Standard OpenMPC | 34.4% | 37.5% | WARN |
| Additional full-parameter stress | 33.9% | 38.2% | WARN |

See [full methodology/raw reports](../benchmarks/1.0.0/README.md). These are
one audio thread's block-budget percentages, not the MPC project CPU meter.
WARN is not PASS and does not establish multi-instance capacity.

Older raw benchmark reports, including intermediate FAIL results, remain
unchanged under [benchmarks](../benchmarks/). Detailed development narratives
and superseded manual plans remain available in Git history rather than the
current documentation tree.
