> **100% AI-generated MPC port and design — tested by the maintainer on a real MPC Live III.**
>
> The MPC integration, interface, artwork tooling, optimizations and documentation were developed with Codex. JC303, Open303, Aciduino, GuitarML models and bundled libraries are existing upstream work by their respective authors; this statement does not claim authorship of them.

# JC303 MPC

An Acid / TB-303-style instrument for **Gen2 MPCs (aarch64), MPC OS 3.10.1+**, based on [midilab/jc303](https://github.com/midilab/jc303). Hardware tests have been performed on **MPC Live III only**.

> **Release status: the public download is still 0.2.3.**
> **The new synthesis, 64-step sequencer, key/scale guide and updated screenshots below are a preview for the NEXT release. They are not included in the current 0.2.3 download.**
> The maintainer confirmed **0.5.0** works on Live III. Installed development candidate **0.7.3** includes independent MODS bypasses, extended LFO/Circuit, Drive Pre-VCA/Disperse and revised navigation/Q-Links, plus CPU optimizations; hardware acceptance is pending. There is no public release date or new binary release yet.

The project/installed name is **JC303 MPC**, the catalog display name is **JC303**. The repository URL and plugin identity remain unchanged.

## Available now: release 0.2.3

[Download the published 0.2.3 installer](https://github.com/303inmyheart/OpenMPC-JC303/releases/tag/jc303-mpc-v0.2.3).

This release contains:

- Open303 mono voice, original filter models, circuit modifications and LFO.
- 16-step sequencer synchronized to MPC transport/tempo, with note/rest, accent, slide, tie, generation and MIDI step recording.
- 37 embedded GuitarML amp/pedal models with gain, dry/wet and background preparation.
- Native MPC preset browser with 12 programs and a dry Init preset.
- Acid artwork, custom knobs, Q-Link mappings and a Sounds-browser tile.
- MPC project/preset state and an OpenMPC Installer-compatible ZIP.

The maintainer confirmed this release on Live III / MPC OS 3.10.1. Gen1/ARM32 and desktop plugin formats are outside this port's support scope.

## Next release preview — not in 0.2.3

The following changes are implemented in local development builds through **0.7.0** (0.7.0 device acceptance pending) and are planned for the next public release:

- **JC303 v0.14 synthesis:** eight filters, TB-303 / Alpha Juno / Phaseshaper oscillators, Circuit square, sub oscillator and filter-shaping controls. ENGINE and FILTER are subpages of MODS.
- **64-step piano roll:** four 16-step pages, touch note/octave entry, a selected-step frame and moving playhead line.
- **Step articulation:** gate, slide, accent, vibrato, tie, mute, hammer and reverse.
- **Timing/editing:** shuffle, fractional gate length, Full/Dotted/Triplet step scale, reverse playback, pitch polymeter, clipboard and undo/redo.
- **Generate length 1–64:** only the active length is regenerated; later stored steps are retained.
- **Key/scale guide:** all 12 roots and 19 scales plus Off. It highlights note rows without changing notes, quantizing input or restricting the generator.
- **Touch feedback:** updates while stopped and playing; persistent switches use the same native recipe as MODS, independently of momentary note/octave triggers.
- **Compatibility:** the original synthesis/timing remain available for old projects. JC7 stores the 64-step pattern and key/scale guide. Older plugin versions cannot read JC7.

The maintainer confirmed the corrected pitch and ARTIC/front-lane touch controls in 0.4.2. Native sanitizer, actual ARM64/QEMU and skin/installer checks are separate from real-device acceptance. Extended stability and release validation remain.

**New in local candidate 0.5.0, for the next release:** four-slot modulation matrix (LFO, envelope, two selectable-source slots), eight source choices and fourteen destinations. Native MODS subpages expose source, destination, amount and status. SEQUENCE → TOOLS adds **Host Transport**, **Note Gate/Hold** and latched **Note Trigger**, plus an automatable STOP trigger with a Q-Link binding. Note modes follow MPC tempo without transposing the pattern; loaded projects wait for a new note. The maintainer confirmed 0.5.0 works.

**Added in local candidate 0.6.0, next release only:** a global MODS master plus independent Circuit/Osc/Filter/four-slot bypasses that retain their settings; LFO start phase, key sync and contour; Accent Sweep and global Reverse Gate. The hardware-Stop MIDI path clears latched note playback without changing the selected mode. Hardware acceptance is pending; see the [0.6.0 test guide](TESTPLAN-0.6.0-DE.md). **New in 0.7.0, next release only:** Drive Pre/Post-VCA and Disperse; six logical function keys; global Setup and a separate Legacy page. Matrix moves beneath LFO, Generate beneath Sequence. Q-Links follow visible knob groups; only Drive Model is an additional selector binding. All switches remain touch controls. Existing IDs and MIDI CC mappings are preserved. See the [0.7.0 test guide](TESTPLAN-0.7.0-DE.md). Preset management, optional sequencer extras and release validation remain on the [roadmap](ROADMAP.md).

Microtuning and additional external MIDI/macro controls are no longer planned. Existing MIDI note input, pad playing, recording and Q-Links remain.

## Screenshots — next release preview

**Every plugin-interface image in this section shows the unreleased development skin. These changes become available with the NEXT public release, not with the current 0.2.3 download.**

These are offline renders of the actual native skin using values queried from the compiled development plugin. They are not MPC screen captures; host fonts are approximated with DejaVu. No D16 code, artwork or reference image is distributed.

![Next release preview: JC303 MPC SYNTH](docs/screenshots/synth.png)

| Next release: MODS | Next release: LFO |
|---|---|
| ![Unreleased circuit controls](docs/screenshots/mods.png) | ![Unreleased native LFO page](docs/screenshots/lfo.png) |

| Next release: ENGINE | Next release: FILTER |
|---|---|
| ![Unreleased oscillator models](docs/screenshots/engine.png) | ![Unreleased filter models](docs/screenshots/filter.png) |

![Next release preview: 64-step piano roll with A Minor guide](docs/screenshots/sequence-next-release-0.7.0.png)

**Next release: 64 steps on four pages. This image shows page 1–16; the other page buttons are 17–32, 33–48 and 49–64.**

| Next release: ARTIC | Next release: SEQTOOLS |
|---|---|
| ![Unreleased step articulation lanes](docs/screenshots/artic.png) | ![Unreleased timing and editing tools](docs/screenshots/seqtools.png) |

| Next release: SEQLIVE | Next release: SEQKEY |
|---|---|
| ![Unreleased live overrides](docs/screenshots/seqlive.png) | ![Unreleased key and scale selection](docs/screenshots/seqkey.png) |

| Next release: GENERATE | Next release: DRIVE |
|---|---|
| ![Unreleased generator with length 1–64](docs/screenshots/generate.png) | ![Development drive page](docs/screenshots/drive.png) |

| Next release: fixed LFO slot | Next release: freely routed slot |
|---|---|
| ![MATRIX1 development preview](docs/screenshots/matrix1.png) | ![MATRIX3 development preview](docs/screenshots/matrix3.png) |

| Next release: SETUP | Next release: LEGACY |
|---|---|
| ![SETUP development preview](docs/screenshots/setup.png) | ![LEGACY development preview](docs/screenshots/legacy.png) |

The interface images show the local development skin through 0.7.0. They are not included in the public 0.2.3 release.

## Installation and use

Download the [published release ZIP](https://github.com/303inmyheart/OpenMPC-JC303/releases/tag/jc303-mpc-v0.2.3), keep it intact and import it into the [OpenMPC Installer](https://github.com/open-mpc/mpc-vst-plugins). It includes the plugin, skin, Init preset, licenses, scripts and checksums. The repository's preview screenshots do not change the contents of that release ZIP.

Enable **Preferences → Enhancements → External Plugin Support** and choose **JC303 MPC** under External VST Plugins. Play from pads, keys or MIDI clips. Overlapping notes slide; velocities of 100+ accent. MODS enables the extended circuit/LFO settings. DRIVE provides the embedded amp/pedal models.

Published-source build instructions are in [BUILDING.md](BUILDING.md). Instructions for testing the upcoming sequencer are in [the German development test guide](TESTPLAN-0.4.0-DE.md). Matching updated source and build instructions will accompany the next public binary release.

## Validation and performance

The published 0.2.3 Store benchmark is **WARN** (p99 16.5%, maximum 24.6%); see [PERFORMANCE.md](PERFORMANCE.md). This historical result does not describe 0.4.2 or the upcoming matrix.

Development builds are checked with native ASan/UBSan/leak tests, actual ARM64 shared-object tests under QEMU, state/timing/scale oracles and skin/installer validation. QEMU checks are not hardware benchmarks. The maintainer then authorized hardware benchmarks of the completed 0.7.0 candidate and optimization comparisons. Local 0.7.3 measures **WARN** in both the standard run (p99 31.5%, maximum 39.0%) and the additional all-parameter stress (p99 31.5%, maximum 44.5%). The baseline stress was FAIL (188.5% / 201.7%). These are per-core audio-block measurements, not the MPC project CPU meter. All baseline and intermediate failures remain in [the comparison report](benchmarks/0.7.3/README.md). No new public release has been published.

## Roadmap and license

See [ROADMAP.md](ROADMAP.md) for published, working-but-unreleased, in-progress and deferred work, including the other planned MPC ports.

Release **0.2.3** is **GPL-3.0-only**. The upcoming v0.14-based release combines **GPL-3.0-only AND AGPL-3.0-or-later**, with file-specific MIT/BSD notices retained. The [development license audit](LICENSE-AUDIT.md) describes that upcoming source; the matching notices and corresponding source must accompany the next binary release. Original authors retain their respective copyrights.
