# OpenMPC JC303 0.2.3 — Gen2

Native 303-style acid synth for **Gen2 MPCs (aarch64), MPC OS 3.10.1+**.
Hardware testing has been performed on **MPC Live III only**.

This release adds Gen2 package metadata, current catalog templates/checks,
neutral visible drive-model names and native overlay model selection.
The normal page frames remain present; the model list opens on a native overlay
above the page. Candidate 5 removes a full-screen touch-consuming Image so that
outside taps can reach native dismissal. The maintainer confirmed open/select/outside-tap-close on Live III / OS 3.10.1. Earlier 0.2.3 candidates are superseded.

The synth, sequencer, 37 embedded drive models, twelve native presets, Acid skin,
Sounds tile and project compatibility are retained. Model order, filenames,
weights, parameter indices, UID and installed folder are unchanged.

## Requirements

- A Gen2 MPC running MPC OS 3.10.1 or later; Gen1 ARM32 devices are unsupported.
- Preferences → Enhancements → External Plugin Support enabled.
- No ROMs, model downloads or network connection needed at runtime.

## Install

Import `JC303-MPC-0.2.3-mpc-aarch64.zip` into OpenMPC Installer. Keep the ZIP
intact. For the official 3.10.1 folder route, copy its
`portable/midilab - VST - JC303 MPC` folder into a drive's `VST2 Plugins` folder.
The archive includes `plugin.json`/`file64`, the native skin, Sounds artwork,
Init .xpl, license notices, scripts, manifest and checksums. Full instructions
are in `INSTALL.md` inside the ZIP. Save the current project before installation.

The instrument remains **JC303 MPC** so existing projects retain its identity.

## CPU result — WARN

The official OpenMPC benchmark ran on a real MPC Live III / MPC OS 3.10.1,
one fast core, 44.1 kHz, 128 samples, ten audio seconds per stage:

- Worst p99: **16.5%** of a core; worst block: **24.6%**.
- Verdict: **WARN**, because the random Q-Link stress stage exceeds the 15% PASS p99 threshold.
- This is below the WARN limits (p99 35%, worst 80%); no FAIL result is hidden.
- These are isolated audio-thread measurements, not the MPC's overall CPU meter.
- Multiple instances and busy projects need additional testing.

Native ASan/UBSan/leak tests, actual ARM64 shared-object tests under QEMU,
ZIP integrity and catalog validation pass. The catalog checker reports zero
warnings; that does not change the separate CPU WARN result.

## Hardware confirmation

The maintainer confirmed this exact Candidate 5 installer during playback on
MPC Live III / MPC OS 3.10.1, including amp operation, overlay layering and
outside-tap dismissal, and authorized release/submission. Root tested.json
records that version separately from 0.2.2. Other Gen2 models are not claimed
as hardware-tested.

Source and full credits: https://github.com/303inmyheart/OpenMPC-JC303
License: **GPL-3.0-only**, with retained third-party license notices.
