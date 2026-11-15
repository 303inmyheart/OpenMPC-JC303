# JC303 MPC 0.7.4 — exact final-release Live III benchmark

Both runs measure the final installed/released stripped ARM64 binary:
`d5ba7dde3800cba43e66593b79797823b3094724b985e39a4b43704365f0ed11`.
MPC Live III, OS 3.10.1 (maintainer reported), new empty project confirmed by the
maintainer, zero loaded VST2 mappings verified, application kept running.

Pinned OpenMPC host a2dd2b78a19c4cd46257c09eb9a485a155ab2968, unchanged
stages/timers/limits, 10 audio seconds per stage, 44.1 kHz, 128 frames, core 4
A76 / performance governor / 2.4 GHz, SCHED_OTHER nice 19. Temporary private
copies only; cleaned after each run. No settings, project or service changes.

| Run | Worst p99 | Worst block | Verdict |
|---|---:|---:|---|
| Standard four-parameter OpenMPC check | 34.6% | 38.3% | WARN |
| Additional full 348-parameter stress | 33.5% | 43.0% | WARN |

The standard result is attached to the installer. The additional stress result
is retained separately, not presented as a replacement for the standard run.
The full sweep stage alone has mean 13.5%, p99 27.2%, max 43.0%; overall stress
p99 comes from the release-tail stage. Each run separately exercises 270 one-shot
triggers. All raw JSON/text measurements are retained as produced; no repeated
runs were selected to hide a failure.

These percentages are audio-thread time against one core's block budget,
not the MPC overall/project CPU meter. PASS requires p99 <=15% and max <=50%;
WARN requires p99 <=35% and max <=80%. WARN is not PASS and does not promise
that every multi-instance/busy project will be free of overloads.

The [0.7.0 baseline](../0.7.0/README.md), intermediate 0.7.1/0.7.2 failures and
[0.7.3 optimization comparison](../0.7.3/README.md) remain unchanged. Baseline
full stress was FAIL 188.5p99/201.7max. The 0.7.4 DSP equations/quality are the same
as 0.7.3; new-instance/Init MODS switches default Off and a Circuit-page SETUP
reminder is printed. Saved project flags and historical migrations are retained.

Native sanitizer, full ARM64 shared-object/QEMU, native UI/geometry and portable
installer/catalog checks pass. The native browser Init is verified all-switches
Off. Current SETUP hardware capture confirms those Off states; earlier hardware
captures cover the shared 0.7 interface. Broader latest-build touch/listening and
extended/multiple-instance testing are not claimed completed by the benchmark.
