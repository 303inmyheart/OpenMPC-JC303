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


## 0.4.0 — 64-step sequencer development candidate

2026-11-14: native ASan/UBSan/leak checks and actual ARM64 shared-object checks
under QEMU pass. Existing legacy voice/clock/loop tests retain sample-identical
audio. New checks cover fourth-page step 64, pitch/octave cells, exclusive
articulations, selected-column views, active-length-only generation, page
clipboard, undo/redo, read-only views, malformed-state rollback, JC6 exact
roundtrip and JC5 legacy-timing migration. A monotonic 64-step cursor sweep with
immediate host echoes passes. Independent sample oracles verify fractional
shuffle/gates, 1% sub-tick gates, 303 IRQ/latency timing and Full/Dotted/Triplet
step scales. Actual VST audio differs for mute, hammer, reverse and vibrato,
confirming these flags reach the modern DSP rather than only the editor.

The final native skin geometry checker passes after the model-overlay and
subpage transformations. Six main function keys remain; SEQUENCE owns
SEQUENCE / ARTIC / SEQTOOLS / SEQLIVE. Native-rendered piano roll, articulation
and generator previews were visually inspected. Existing VST indices 0–85 are
unchanged; new API is append-only. Installer/portable catalog validation returns
zero warnings. The install ZIP contains no user reference image, source, local
logs or toolchain files. Key/scale highlighting remains on the roadmap.

Installed under standing user authorization: the old 0.3.1 plugin folder was
backed up locally; 286 plugin-specific files were staged, verified and replaced.
MPC.settings stayed unchanged during file replacement. The MPC application was
restarted; active/running and a new process were verified. Installed binary hash
matches the stripped package binary:
`7728e03380c0f94cb33dd9cc3a532806b7c7854180d0630707c63471bbefac7a`.
No device GUI/touch/hearing acceptance or real-device functional harness is
claimed for this candidate. See TESTPLAN-0.4.0-DE.md for the hands-on test list.

No formal/store performance benchmark was run or attached. It is reserved for
the final release build at the user's explicit instruction. This development
candidate is local/installed only; it is not a published GitHub/store release.


## 0.4.1 — touch feedback, playhead line and key/scale guide

Installed on Live III after native sanitizer, actual ARM64/QEMU VST, final skin
and catalog checks. 354 plugin files were backed up/staged/hash-verified;
MPC.settings stayed unchanged during replacement. The application restarted and
returned active/running. Installed stripped binary SHA-256:
`81709a093a9d11cecb64637ccc50e9b267ff622d87411c9b20d6795fced0c997`.

The old 0.4.0 ARM64 binary fails the offline host-model regression: momentary
UI actions remain armed without audio rendering. A separate baseline run also
shows global parameter-screen rebuilds during playback. The opt-in async idle
feedback patch returns action values and derived cells without audio blocks;
callbacks run on the UI worker, and hidden legacy clock/tile polling is off.
Stopped-input, Play/Stop persistence, canonical host-cache values, octave
72/60/48/24, duplicate touch-lift writes and deliberately blocked UI callbacks
pass native/ARM64 checks. These are host-model tests, not replayed MPC touches.

All 12 roots and 19 scales plus Off match independent interval definitions.
The guide does not alter stored notes or rendered audio. JC7 stores the guide;
JC6 imports preserve 64 steps with guide Off. All prior parameter IDs remain.
The moving playhead uses existing read-only indicators. The user confirmed that
pitch now fits, but reported persistent flag touch problems on ARTIC and the
three SEQUENCE lanes. 0.4.1 therefore does not have full touch acceptance.

## 0.4.2 — persistent switch correction (local candidate)

Persistent grid cells now use the device-tested MODS toggle recipe: native
Button handles touch, parent Mouse Down selects Q-Link, Enter toggles. Momentary
note/octave triggers retain their separate trigger/debounce behavior. The old
shared trigger recipe on persistent cells could undo a touch or cause two
related edits. GATE now always clears TIE when explicitly setting note/rest,
so a tied rest can be restored without losing its stored pitch.

The generated-skin test compares switch actions to MODS, resolves all 176 cell
centers on ARTIC/SEQUENCE, and checks ON and OFF against the real VST. It checks
TIE -> tied rest -> GATE -> restored note explicitly. Native ASan/UBSan/leak and actual ARM64/QEMU shared-object tests pass, as do
final native geometry and skin-to-VST input checks. Installer validation reports
zero warnings, with all file hashes verified. The installer privacy scan passes.
No public release or formal/store benchmark is part of this candidate.

Latest user instruction: do not install 0.4.2 or restart the MPC application
until their explicit Go. Device remains on 0.4.1. Hardware touch acceptance of
0.4.2 is pending; no offline test is described as a real touch test.


0.4.2 completed local verification: the persistent switch action list matches
MODS, all 176 flag-cell hit targets resolve to their printed row/column and
support ON/OFF, and GATE/TIE restoration is covered in native/ARM tests. Final
installer: JC303-MPC-0.4.2-mpc-aarch64.zip under ignored build/releases; SHA-256
`df36aa0e80c6058a8f539c0700695e71c2e033f946ea06ec399310aaceea8294`.
No Desktop copy, public push/release, device install, device restart or benchmark
was performed for 0.4.2. Installation is held for the user's explicit Go.


0.4.2 subsequently installed after the user's explicit Go. Previous 0.4.1
plugin backed up locally; 354 plugin files staged and hash-verified. Settings
unchanged during file replacement. MPC application restarted, active/running
and new process confirmed. Installed stripped binary matches the package:
`0a03d95fe187ac43851d51b2cca7ef112b75c490f6aa5d5745bf7feece37ae2e`.
Actual touch/hearing acceptance remains pending. No other plugins, projects or
global settings were manually changed. Future installs still require user Go.


0.4.2 user acceptance update: the user confirmed that the corrected persistent
ARTIC switches and SEQUENCE Slide/Accent/Vibrato touch controls work after the
approved installation. This confirms the reported switch regression, not a
claim of testing every 64-step/record/import scenario or completing v0.14 parity.


## 0.5.0 local candidate — installed; acceptance pending

Native ASan/UBSan/leak and actual ARM64 shared-object/QEMU suites pass. Matrix
checks cover 8 source choices × 14 targets for finite output, bipolar routing,
MODS/legacy bypass, transient neural-drive wet mix, transactional JC8 state and
JC7 migration. Note-mode checks cover sample-offset starts, MPC-tempo timing
across blocks, non-transposing triggers, overlapping held notes, release/latched
behavior, host Stop, explicit STOP, API automation availability and fresh-note
waiting after state restore. Existing legacy/reference, all 37 model checks,
64-step/scale oracles and Stop/Play touch-feedback regressions still pass.

Final native skin checks validate matching subtab/Q-Link maps, all fourteen
route choices on each matrix page, the STOP hardware binding and all 176 flag
cell centers and reversible ON/OFF actions. Offline previews were regenerated.
Installer/catalog validation reports zero warnings; privacy and file hashes pass.
These are offline tests, not real-device touch/automation acceptance.

At the user's explicit request and subsequent standing authorization, 504 plugin
files were backed up locally, staged and hash-verified on Live III. Settings were
unchanged during replacement; MPC application restart completed and its service
is active/running. Installed binary SHA-256:
`0bebc08a3efba0f2d2ed9887a9a6019be55209dfef8aca3a10ef39a8b022440b`.
Installer `JC303-MPC-0.5.0-mpc-aarch64.zip` SHA-256:
`80abec367410a379a3709e274f538adbf898c1ef4dfb7cc21a5dba5b8590eb9a`.

Hardware hearing, touch/Q-Link and MPC automation acceptance is pending; follow
[the German test guide](TESTPLAN-0.5.0-DE.md). Public release remains 0.2.3. No
source push/public binary release, Desktop copy, full device reboot or formal
Store/CPU benchmark was performed for this candidate. Standing deployment
authorization now lasts until the user revokes it, within our plugins only.


## 0.6.0 — independent MODS and extended LFO/Circuit

The user confirmed the installed 0.5.0 works. Authorized scope for 0.6.0 is
phase 1 + phase 2 only; Pre-VCA/Disperse, preset import/export and sequencer
extras were not implemented.

Native sanitizer regression suites and supplemental final-source checks pass,
including independent Circuit/Osc/Filter/four-slot bypass, retained settings,
master bypass of advanced filter shaping, and protection against reactivating
a bypassed LFO route through LFO Depth modulation. LFO phase only affects
key-synced starts; positive/negative contour and Accent Sweep/global Reverse
Gate affect captured audio. JC9 roundtrip, malformed rollback and JC8 enabled-
page/neutral-effect migration pass. The real VST MIDI fixture handles MPC-style
CC123 while host transport is already stopped, clears the latch to Waiting for
Note, and verifies immediate CC120 silence. This is simulated host MIDI, not
proof of an actual hardware Stop press in the MPC app.

After the final master-filter gate correction, supplemental ASan/UBSan checks
were repeated and the exact rebuilt ARM64 shared object passed the full suite
under QEMU. Existing voice, all 37 neural models, legacy migration, 64-step
editing/clock/scale and UI feedback regressions pass. Final native skin geometry
has no findings; matrix/area Q-Link bindings and all 176 flag-cell ON/OFF hit
targets pass. Offline screenshots were reviewed.

The OpenMPC installer/catalog validator reports zero warnings. Archive privacy,
required installer scripts, checksums and tested ARM64 binary receipt pass.
No Desktop copy, public source push/release or formal Store/CPU benchmark.

Installed under the user's standing authorization: prior 0.5.0 plugin backed up
locally, 505 package files staged and hash-verified. Settings unchanged during
file replacement. MPC application restarted and active/running confirmed;
installed binary SHA-256 matches the package:
`b9bc92471670095fa1048ac7dc6511febc89811928c67654bf6d7388f09c1244`.
Installer SHA-256:
`99135a4b3ac0287df35ae92f2f97729f84025d72f9f47bc7d3c63ec8dc88313e`.
Actual 0.6.0 hearing, touch/Q-Link and hardware Stop acceptance is pending; follow
[the German test guide](TESTPLAN-0.6.0-DE.md). Public release remains 0.2.3.


## 0.7.0 — Phase 3 and knob-focused navigation (2026-11-15)

Implemented Pre/Post-VCA routing and original eight-stage Disperse without
changing IDs 0–614. JC10 appends Drive Position/Disperse; JC9 migration defaults
to Post / Off. Legacy synthesis explicitly falls back to Post. Route changes
use a short fade with only one neural path running.

Final native ASan/UBSan/leak tests and actual ARM64/QEMU shared-object tests pass.
A redundant full native recompilation was interrupted; final native validation
reused current-source sanitizer DSP objects and rebuilt generated data, the
final wrapper and both test hosts. All behavioral suites ran again. Published
CC20–35 assignments and NRPN pass on both architectures. No audio failure is
hidden by the interrupted build. The earlier migration fixture was corrected to
replace the entire version line now that JC10 has a two-digit version number.

Independent checks cover eight-allpass impulse phase/energy, zero bypass and
clean reactivation. Real wet paths cover three neural models, panic, disabled
dry preservation and transactional JC10/JC9 state. A separate nonlinear
insert-location oracle verifies that changing Volume scales Pre-VCA output
proportionally, while Post-VCA intentionally fails that proportionality test.
All prior legacy/reference, 37-model, matrix, bypass, sequencer/clock/scale and
Stop/Play UI regressions pass.

Six function-key groups, all knob-only bindings plus the Drive Model exception,
RowTopDown bank order and complete native knob/name/value bounds pass. Existing
176 flag-cell touch ON/OFF tests remain passing; navigation does not alter their
native action definitions. Drive's native overlay retains focus ownership,
outside-tap dismissal and placement above page Q-Link rectangles. Geometry
reports no findings. Offline screenshots were reviewed. Actual MPC Q-Link bank,
frame, overlay and listening acceptance is still pending.

OpenMPC portable installer/catalog checks pass with zero warnings. Required
scripts, exact tested binary receipt, checksums and privacy scan pass. Installer:
`JC303-MPC-0.7.0-mpc-aarch64.zip`, SHA-256
`89b7c702414a5a3efe95ed83f94da64facd920960bc22cf415fb3c23892441fe`.

Installed under standing authorization: prior 0.6.0 backed up locally; 511 plugin
files staged/hash-verified; settings unchanged during replacement. MPC app
restarted, service active/running and installed binary hash verified:
`e2beacd52606c00c00838ae703e169e6c6a1fe3a208764850b05ecf48bea8d6e`.
No manual settings/project/unrelated-plugin changes, full device reboot, Desktop
copy, public source push/release or formal Store/CPU benchmark.

See [test guide](TESTPLAN-0.7.0-DE.md) and [exact Q-Link table](QLINKS-0.7.0-DE.md).


## 0.7.0 benchmark baseline (user explicitly requested)

The user explicitly authorized benchmarking the finished installed candidate,
overriding the earlier hold for this exact binary. The new empty project was
maintainer-confirmed. Official default run: WARN (p99 34.5%, max57.4%). Additional
full-parameter stress: FAIL (p99 188.5%, max201.7%). All results and 21 fixed-
configuration repeats are preserved in [the report](benchmarks/0.7.0/README.md).
Same installed/installer binary, hash verified. Temporary copies cleaned; MPC
app/services/settings/projects were not modified. No report was attached to a
new installer and no public release/push was performed. Baseline must remain
unchanged during subsequent optimization.


## 0.7.3 — CPU optimization and verified deployment (2026-11-15)

Accumulated changes since 0.7.0: coalesce square waveshaper table requests,
apply only the active engine's final requested table before rendering, collect
derived sequencer display differences in the idle notification worker, replace
618-key linear lookup with an immutable allocation-free index, and skip unused
TeeBee/Diode cutoff coefficient rebuilds while maintaining sounding/incoming
models during switch fades. IDs, state, UI, quality and undo depth are unchanged.

Native ASan/UBSan/leak host and full behavior suites pass. Interrupted earlier
runs were retained under private interrupted filenames; the final current-source
host/voice binaries were run to completion again. Exact final ARM64 shared-object
host and full behavior suites pass under QEMU; tested receipt matches raw binary
SHA-256 `c6fc0ade6c59be9f750ab71c21a2b08c3868d0b81b33023d5e3051f71903fd39`.
Native and real Live III old/new comparisons each cover **917504 bit-identical
samples** against installed 0.7.0: drive/bypass/step/scale setter bursts, all eight
filter models, moving cutoff/bias, Hi-Fi/FM/saturation and live model changes.
Filter cases include note-off/retrigger segments. These finite comparisons do
not replace listening or touchscreen acceptance.

Native grid 176 flag-cell hit/ON/OFF tests, knob-only navigation/frame checks,
Drive popup focus/dismissal and geometry pass. Final TUI.json and Q-Links.json
are byte-identical to installed 0.7.0. No interaction/layout change was intended.

Explicitly authorized real Live III measurements use the unchanged pinned
OpenMPC host and thresholds. Standard: **WARN, p99 31.5%, max 39.0%**. Additional
full 348-parameter stress: **WARN, p99 31.5%, max 44.5%** (baseline0.7.0 FAIL at
188.5% / 201.7%). Full sweep stage alone: mean13.4%, p99 26.4%, max44.5%; overall
p99 comes from the release tail. All raw baseline/intermediate failures remain
in [the complete comparison](benchmarks/0.7.3/README.md). Per-core block-budget
CPU figures are distinct from the MPC project meter. WARN remains WARN.

OpenMPC Installer/catalog/checksum/privacy validation passes with zero warnings.
Installer `JC303-MPC-0.7.3-mpc-aarch64.zip`, SHA-256:
`cde5060fef5b8abb51714a401ff743a60cd24e26656c8642b65ee5c77d3d8fd3`.
Package carries the conservative additional full-stress WARN report.

Installed under standing authorization: 0.7.0 backed up locally, 511 files
staged and hash-verified, settings unchanged during replacement; only the MPC
application restarted, active/running confirmed. Installed stripped SHA-256
matches the package and both hardware benchmark runs:
`1354f721127c8c945f450958a539a7852e3d27002717ae8b476fc000feb34de9`.
No global settings/project/unrelated-plugin writes, full device reboot, Desktop
copy, public source push or release. Hearing/touch acceptance remains pending;
see [the short German test guide](TESTPLAN-0.7.3-DE.md).


## 0.7.4 — MODS Off defaults and SETUP reminder (2026-11-15)

New instances and the exported MPC-browser Init now default the global MODS
master, Circuit/Osc/Filter and all four matrix/LFO-slot enable switches to Off.
Generated parameter defaults and engine initialization agree. Loading Init after
a factory sound also resets the seven page/slot switches; existing non-Init
factory sounds preserve their enabled routing. Saved JC9/JC10 switch settings
are restored exactly, and older chunks without SUBMODS still migrate with their
historically enabled pages. No DSP equations, IDs, state format or routing change.

MODS / CIRCUIT now prints **ENABLE MODS MASTER IN SETUP** beneath its status
readout. Preview reviewed: the message is outside control/value/Q-Link regions.
Native TUI actions and Q-Link maps are byte-identical to 0.7.3. Geometry, 176
flag-cell touch/action regressions, knob-only banks and popup focus checks pass.
Actual MPC visual/input acceptance remains pending.

Native ASan/UBSan/leak and full ARM64 shared-object/QEMU suites pass, including
explicit raw-instance defaults, Init reset, LFO factory preservation, mixed-page
state roundtrip and JC8 historical migration regressions. DSP oracles explicitly
enable submodules in their shared legacy fixture so they still exercise active
mods; the separate default checks opt out of that setup. Browser Init export
checks all eight real VST switch values before serializing the .xpl.
Raw tested ARM64 SHA-256: `f1b6ede599b0bf69a0cb9c81d077a4dbaa6b6f28a6093eaa0b7a6d509158d7ec`.

OpenMPC Installer/catalog/checksum/privacy validation passes, zero warnings.
Installer `JC303-MPC-0.7.4-mpc-aarch64.zip`, SHA-256:
`3e5607ff51c8bb0daa63755c1fe8eedf1788b4b364df9c00a3553918dddffeb2`.
No 0.7.3 CPU report is attached to this changed binary. A matching final release
benchmark remains required; no new development benchmark was run.

Installed under standing authorization: previous 0.7.3 backed up locally, 511
plugin files staged/hash-verified, settings unchanged during replacement, MPC
application restarted and active/running confirmed. Installed stripped SHA-256:
`d5ba7dde3800cba43e66593b79797823b3094724b985e39a4b43704365f0ed11`.
No global settings/project/unrelated-plugin changes, full device reboot, Desktop
copy, public push or release. Hearing/touch acceptance pending; see
[the German test guide](TESTPLAN-0.7.4-DE.md).
