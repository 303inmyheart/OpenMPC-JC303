# Verification — updated 2026-11-13

## 0.2.3 Gen2 catalog release candidate

Native ASan/UBSan/leak and actual ARM64 shared-object tests under QEMU pass.
The official OpenMPC bench.c at a2dd2b78a19c4cd46257c09eb9a485a155ab2968 ran
against this exact ARM64 binary on MPC Live III / MPC OS 3.10.1, core 4,
nice 19, 44.1 kHz / 128 samples, ten audio seconds per stage. The only helper
changes set its own affinity/priority; benchmark behavior is unchanged.
WARN: p99 16.5%, worst block 24.6%; full log and matching binary SHA-256 in
benchmarks/0.2.3/bench.txt. Temporary files were removed; MPC was not restarted.

The current official release.py / catalog_check.py package passes --catalog,
--require-portable, --expect-id and --expect-repo with zero warnings. Gen2/ARM64
scope is declared; only Live III has hardware evidence. The maintainer confirmed installed Candidate 5 of 0.2.3, including the native
model-overlay dismissal, and authorized submission. Root tested.json records the maintainer’s confirmed
0.2.2 test (including the Sounds tile); a separate 0.2.3 record documents the newly confirmed candidate.

Visible drive names now use neutral aliases; model order/files/weights stay
unchanged. MODELS.md retains the original credits and names. No DSP behavior
change is intended in this release candidate.

During maintainer playback of 0.2.3, read-only process samples showed the same
busy audio worker at approximately 7.3% of one core without amp and 12.2% with
amp. Other audio threads remained similar. This is a whole-project observation,
not an isolated benchmark or a complete installed-plugin smoke test.

The maintainer found a Q-Link frame drawn above the open model list. Candidate 2
removed the page-wide DRIVE frame and was rejected. Candidate 3 used child-level
hideQLinkBounds; the maintainer reported that the frame still appeared above the
menu. Candidate 4 moved selection into a real native overlay; the maintainer
reported that layering was nearly right but the menu could not close. Candidate 5
keeps the real overlay and paints its full-window artwork as backgroundData,
removing a full-screen Image child that could intercept outside taps.

The overlay contains a known native KnobOverlay for Focus Lost dismissal and
37 original model-choice buttons inheriting the model Data handle. All page
Q-Link rectangles are unchanged, and the field action is Show Overlay on model
parameter 60. Structural checks confirm the bindings, choices, background and
absence of a full-screen Image touch target. The maintainer then confirmed the installed open/select/outside-tap-close
behavior and authorized submission of Candidate 5. The ARM64
DSP binary and its measured SHA-256 are unchanged across skin-only candidates.

0.2.2 OpenMPC release ZIP passed portable catalog validation, integrity and
all archive checksums; the only warning is the missing public source_repo.

## 0.2.2 Q-Link frames, step selection and Sounds tile

Native ASan/UBSan/leak checks and actual ARM64 .so tests under QEMU pass.
Only skin/packaging and plugin version metadata change; no DSP changes.
All six generated pages and open model chooser are reviewed offline. Q-Link
rectangles derive from complete placed native controls, plus printed headings
and padding. On SEQUENCE the enable switch moves beside TIE and every column
outline starts below the entire step grid. Q-Link assignments remain the same.

Step on/off artwork has the same dark interior; the selected state adds a 3 px
acid-green outline. The actual native tile strings were rendered in a QA preview;
interior and selected-border pixels and clearance from Q-Link frames were checked.

The 270x110 native tile is Plugin Skins/browser_images/soundsmode.png. Init .xpl
contains the actual fresh plugin chunk, exported through VST2 effGetChunk. JUCE
bank/base64/UID are checked, and its decoded chunk restores a wet Rat Bass instance
to dry Init with sequencer off. Default.xpl is removed. Package checks require the
tile dimensions and Init .xpl. Both ship inside the standard portable plugin folder.
The native twelve-program bank is unaffected. Actual MPC UI/banner behavior needs
user confirmation after installation. No device access or device writes this turn.

0.2.1 installer package: portable catalog check, archive integrity and every
SHA-256 entry passed. Desktop ZIP: JC303-MPC-Live-III-0.2.1-OpenMPC.zip
(5,246,564 bytes; SHA256 a137ec081d8630d61a351bad4bc26d39dbc7e5b30f38510d2eb82e3bab6880e4).

## 0.2.1 Acid skin and inference optimization

Native ASan/UBSan/leak checks and actual ARM64 shared-object tests under QEMU
pass. The new fixed-rate recurrent path matches LinInterp(1) sample for sample
on all 37 models. The scalar oracle now also covers Blackstar model 16.

The actual ARM64 host/voice test executables passed on the user's Live III
against a private shared object under /tmp. CPU and audio comparisons were
performed on-device; PERFORMANCE.md and benchmarks/live-iii-0.2.1.json record
the method/results. Blackstar's captured 262,144 samples are bit-identical.
This tests isolated copies, not the installed plugin or the new UI on-screen.

All six skin pages and the open drive chooser were visually inspected.
Generated knob strips have 128 frames of 92x92 (11776 px tall, below the native
image limit). Native parameter indices, action types and Q-Link order are kept.
Live value labels use green text on dark recessed plates. Selector and toggle
states use matching off/on artwork. Open-popup overlap warnings are expected;
no other layout bounds/touch errors were found.

The isolated hardware tests used temporary analysis copies. No installed plugin,
project or MPC settings were changed, no service was restarted, and temporary
analysis files were removed afterwards. The new skin needs user installation/testing.

0.2.0 packaging: native ASan/UBSan and actual ARM64 shared-object suites passed.
OpenMPC release ZIP passed portable catalog, integrity and checksum checks;
only the expected missing public source_repo warning remains. Installer ZIP:
JC303-MPC-Live-III-0.2.0-OpenMPC.zip.

User feedback after installation of 0.2.0: "so klappt alles". This confirms
basic device operation including the browser additions; no measured CPU or
long-duration stability results were provided.

## 0.2.0 drive and native preset browser — offline

User feedback: the 0.1.0 build works wonderfully on their Live III. The agent
has not connected to the device. The following 0.2.0 checks are offline only:

- Native ASan/UBSan plus the actual ARM64 shared object loaded under QEMU.
- All 37 embedded models load, warm up and render finite output; rapid switch
  cancellation, switching during load, exact bypass and destruction during load.
- Independent scalar LSTM reference from PyTorch weights for models 0, 1, 4, 36:
  native maximum differences 0.000000664, 0.00000366, 0.00000317, 0.000000805.
- The wet path changes the synth output through the actual VST2 plugin.
- Twelve named VST programs, Init on fresh load, drive/sequencer off in Init,
  selection and edited state round-trip; older chunks load as dry Init.
- Existing synth, clock, sequencer, MIDI, Q-Link echo and migration regressions.

The SIMD oracle/sanitizers exposed an out-of-bounds input-kernel read in pinned
RTNeural's two-input LSTM. The loop now stops at in_size. See VENDORED.md.
Six page previews and the three-column model chooser were visually inspected;
chooser captions use DejaVu artwork for legibility. The layout checker reports
expected open-popup overlaps with controls underneath; the generator places
popup controls on top. Runtime focus/touch, native MPC browser opening and model
selection still require device verification. No hardware CPU/latency benchmark.

## 0.1.0 sequencer — 2026-11-13, offline

The original 22 parameter indices remain unchanged; controls/readouts for the
sequencer are appended (59 total). New chunks use state version 3 and retain
all 16 steps plus editing/generator settings. Versions 1/2 load as sound-only
states with the sequencer off; legacy slide-duration migration remains tested.

Passing native ASan/UBSan checks include:

- A separately written beat-grid oracle against the 96-PPQN host clock, with
  variable buffer lengths, loop endpoints inside blocks, mid-song starts,
  tempo changes, small backwards PPQ jitter, and Stop.
- Explicit expected note/velocity/tick events for accents, slide overhang and
  tied rests, including ties across the last/first step. Normal gates last the
  intended 12 ticks (half a step), rather than expiring one tick early.
- Exact VST2 audio against an independently scheduled Open303 voice, both
  through a pattern and through a non-aligned host cycle: maximum error 0.
- Full pattern project round-trip, MIDI step record, sustain-edge rest entry,
  deterministic generator limits and editor-notification echo stability.
- Edit-cursor sweeps through all 16 steps in both directions with host echoes.

The same suite is run against the actual ARM64 shared object through QEMU.
Shared `poc/songpos` host tests and desktop VST3 44.1/48/96 kHz tests pass.
Five skin pages are generated with no layout errors. The new sequence UI,
actual host transport and CPU load still require Live III verification.

0.0.3: reproduced LFO waveform reversal during a strictly increasing absolute
knob sweep. Accepted-value host echoes overwrote the knob gesture's direction
history. The wrapper now recognises acknowledgements of values it actually
reported and preserves gesture history only for those acknowledgements.
Regression: all six waveforms in order, forward/reverse sweeps, wheel clicks,
no host echo, synchronous echo and delayed echo. Native sanitizers and ARM64
QEMU tests pass. Genuine exact touch/automation writes retain their semantics.

Slide Time now spans 2–1000 ms. The default retains the previous duration.
State version 2 reads version 1 and rescales stored slide values from 2–360 ms.
The regression checks the 1000 ms maximum and a legacy 120 ms state. Existing
external normalized automation still follows the newly extended parameter scale.

User device feedback: MPC Live III, MPC OS 3.10.1 or newer, version 0.0.1.
Installation and basic operation work. The user reports that MODS, filter choice,
and LFO selections do not visually follow Q-Link changes reliably. No device CPU
benchmark or agent-controlled device session has occurred.

0.0.2 correction: the VST2 wrapper now reports accepted stepped values back to
the host when it rounds a fractional Q-Link/wheel write. The regression first
failed with "host cached fractional Q-Link value; touch state is stale" before
the fix. It checks MODS on/off, filter forward/reverse, LFO wave and destination,
an exact touch selection, no callback inside setParameter, and a host echo that
must not cause a notification loop. Filter Drive's inactive/active display is
also checked. This correction requires device confirmation after installation.

Native VST2 tests (`python3 tools/test.py`, ASan + UBSan, leak detection,
halt on first runtime error) pass:

- Framework host test: identity, names, continuous/enum parameters, Q-Link
  and wheel option stepping, independent instances, MIDI audio, accumulation,
  MPC STOP's parameter-0 toggle, chunks, CC/NRPN and concurrent host calls.
- Direct Open303 reference comparison for all three filters: note at sample
  37, subsequent blocks of 1, 31, 96, 127, 128, 257 and 511 samples, with a
  quiet output level to expose 16-bit quantisation. Max error < 2e-9.
- Instance isolation, invalid/truncated project chunks, repeated and overlapping
  notes, MIDI all-sound-off and restart, and 54 filter/LFO combinations.

ARM64 (`python3 tools/test_arm64.py`) passes the framework and behavioural
tests against the **actual shared object loaded with dlopen**, under QEMU.
These are load/ABI/functional tests; they do not measure Live III performance.

Shared toolchain regressions: existing int16 `poc/inputprobe` VST2 host test
passes; float desktop VST3 tests pass at 44.1, 48 and 96 kHz, including state.
The VST3 build is a test artifact and is not in the installation package.

Three generated skin previews were visually inspected; layout generation
reports no touch overlap, edge or Q-Link errors. Runtime values are drawn by
the MPC host and require device verification.

Pending Live III checks:

1. Scan/load JC303 MPC on MPC OS 3.10.1+, show all three pages.
2. Play pads and sequenced notes; verify slide, accents and STOP.
3. Change all filter/LFO options and Q-Link controls.
4. Save/reload a project; verify every parameter and multiple instances.
5. Benchmark render and parameter sweeps, especially Square Drive / MODS
   changes, which regenerate the upstream square wavetable synchronously.
   Optimisation is required if those operations exceed the audio deadline.

No device was accessed, installed or restarted by the agent. The user feedback
above is not a full hardware verification of this development package.

Packaging correction: the initial folder-copy ZIP was unsuitable for the OpenMPC
desktop installer (missing install.sh). The default now uses the official
release.py portable structure. Version 0.0.1 passes catalog_check.py
--require-portable, ZIP integrity, ARM64 metadata and all archive SHA256 checks.
The only checker warning is the missing public source_repo for this local port.
The DSP binary is unchanged from the tested milestone-1 build.

User-reported CPU: 30–33% on Live III with synth + sequencer +
BlackstarHT40 GainKnob SM57mic enabled (0.2.0). Instance count and meter semantics
not independently verified. Subsequent isolated benchmark results are documented
in PERFORMANCE.md; they do not replace the MPC UI meter reading.

## 0.3.0 — upstream v0.14 synthesis, stage one (2026-11-14)

- Native ASan/UBSan with leak detection and ARM64 actual shared-object tests under
  QEMU pass. All existing voice, sequencer, 37-model neural-drive, preset-browser,
  host echo/rotary and one-second slide migration checks remain passing.
- All three old filter modes load a v4 fixture into Legacy and produce
  sample-identical audio. Old VST definitions 0–64 are unchanged; the old implicit
  popup flag is explicitly retained at 65. New settings append at 66–85.
- 24 combinations of three new oscillator models and eight filters render finite,
  audible output; quality/morph edits are exercised. The three oscillator
  families produce different captured signals. Existing host-sequencer notes are
  routed into the selected new engine; an engine switch silences inactive tails.
- JC303-MPC-5 round-trips all new sound settings and exact captured audio.
  Non-finite/truncated modern state is rejected transactionally. Versions 1–4
  restore Legacy. New Synth initialization, real Init .xpl and wet-to-dry browser
  restore are verified. Old program sounds deliberately select Legacy.
- Built native skin reviewed offline: eight pages, new ENGINE/FILTER pages,
  custom Acid knobs, readable selector headings and preserved native Drive
  overlay. Final skin checker has zero findings. Offline geometry is not an MPC
  screenshot; touch/Q-Link and listening acceptance remain pending.

### Isolated Live III diagnostic, not an MPC-app listening test

Candidate ARM64 binary loaded from a private temporary directory; helper pinned
at normal scheduling / nice 19 on A76 core 4. 44.1 kHz, 128-frame blocks, one held
note re-struck every half-second; 2 seconds of audio per case. MODS on, cutoff
50 %, resonance 80 %, no neural drive, no sequencer. Thread CPU time excludes
pre-emption and is expressed as a percentage of the 2.902 ms block budget.

| Filter / variant | Mean % | p99 % | Max % |
|---|---:|---:|---:|
| TeeBee | 4.879 | 5.205 | 6.944 |
| Spicy Standard | 7.583 | 7.969 | 9.104 |
| MissBehave | 25.032 | 28.760 | 29.604 |
| JuneBug | 21.410 | 22.379 | 23.474 |
| Wasabi | 18.109 | 19.183 | 20.037 |
| Hornet | 19.855 | 21.314 | 21.444 |
| SemiSweet | 20.168 | 20.761 | 22.087 |
| Oddity | 19.922 | 20.570 | 21.776 |
| Spicy Hi-Fi | 10.624 | 10.983 | 12.149 |
| Alpha Juno / Spicy | 7.970 | 8.180 | 9.335 |
| Phaseshaper Formant / Spicy | 7.143 | 7.356 | 8.521 |

This is a short diagnostic, not the formal release benchmark, nine-instance
stress test or a guarantee that every edit is crackle-free. Do not reuse 0.2.3's
published CPU verdict for this binary. Strong resonance can exceed unity output;
no new limiter is added. Temporary helper/library were removed. MPC application,
settings and installed plugin were untouched during the diagnostic.

Installation under the user's standing authorization: the previous 0.2.3 plugin
folder was backed up locally, 179 package files staged and hash-verified after
replacement. MPC.settings stayed unchanged during the file update. The MPC
application was restarted and returned to active/running. Other plugins were
not updated. Hearing/touch/GUI acceptance of 0.3.0 remains pending. Installer
checksums, portable catalog check (zero warnings), binary privacy scan, real Init
preset and native skin checks passed. No Desktop copy or public release was made.
The exact candidate installer and private deployment record remain in build/.

## 0.3.0 live acceptance and 0.3.1 navigation

The user reported that 0.3.0 appeared to work, including enabling the amp near the
end. A read-only live recording captured 229 samples over 234.7 seconds. Whole
MPC process CPU averaged 54.2% of one CPU core, p95 68.4%, maximum sampled interval
82.7%; busiest AudioWorker averaged 25.1% and peaked at 45.2%. RSS stayed 385.6 MiB,
hottest sampled sensor 39.8 C. No matching audio warnings appeared in the service
journal. Amp activation time/model and parameter stages were not marked, so no
controlled amp comparison is claimed. This is NOT a Store benchmark or individual
audio-block timing. See docs/performance/jc303-0.3.0-live-2026-11-14.md and CSV.

0.3.1 changes only navigation/artwork/version metadata: ENGINE and FILTER become
MODS subpages 2 and 3. The control definitions and Q-Link bindings are unchanged;
only their native tab/subtab addresses change. Six main keys remain. All parameter
IDs and DSP/state code are unchanged. Native ASan/UBSan and actual ARM64 VST/engine
tests pass. Native skin structure checks confirm unique matching tab/Q-Link
addresses; the existing overlay and Init preset are preserved. Offline rendered
pages reviewed. Actual subpage navigation still needs user acceptance.

0.3.1 was explicitly approved for installation: 179 files staged and hash-verified,
previous plugin backed up locally and settings unchanged during file replacement.
MPC application restarted and returned to active/running. Subpage acceptance is
pending. No benchmark result is attached to this development installer. The user
requires the formal Store benchmark only at the final release-build milestone;
a prematurely started development run is not used as release evidence.
