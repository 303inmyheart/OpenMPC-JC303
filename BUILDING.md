# Build JC303 MPC

The supported target is Gen2 MPC / ARM64, MPC OS 3.10.1 or newer.
Hardware tests are recorded for MPC Live III only.
DSP/model dependencies are vendored; no JUCE checkout, desktop GUI or Steinberg
SDK is needed for this port. Builds use a separate, pinned OpenMPC toolchain.

## Checkout

```sh
git clone --branch mpc-live-iii https://github.com/303inmyheart/OpenMPC-JC303.git
cd OpenMPC-JC303
git clone https://github.com/open-mpc/mpc-vst-plugins.git ../mpc-vst-plugins
git -C ../mpc-vst-plugins checkout ef05a26c6e59f1bfd3ef2023e6a9d8b371b61310
git -C ../mpc-vst-plugins apply "$PWD/patches/mpc-float32.patch"
git -C ../mpc-vst-plugins apply "$PWD/patches/mpc-async-revision-cache.patch"
git -C ../mpc-vst-plugins apply "$PWD/patches/mpc-engine-program-bank.patch"
```

Use that commit and apply the three included patches once, in that order, to a clean checkout. The
patch supplies the float engine ABI, stepped Q-Link feedback and host loop
metadata this port expects. It includes the new audio_format.h file. The second
patch supplies the async UI path and opt-in ASYNC_IDLE_NOTIFY: stopped-input
feedback and momentary spring-back run without requiring audio blocks. Revision
cache updates and atomic pending flags prevent stale or duplicate delivery.
Native test hosts allow async callbacks to drain; callback counters are atomic.

Release templates/checks are pinned separately to the current catalog revision:

```sh
git clone https://github.com/open-mpc/mpc-vst-plugins.git ../mpc-vst-plugins-release-tools-20261127
git -C ../mpc-vst-plugins-release-tools-20261127 checkout 132b0db05c3903f1aeb37a5021a0b556811eb55a
```

This keeps the tested wrapper patch/base stable while using current catalog
installers, manifest generation and validation. `release.json` records that pin.
`MPC_RELEASE_TOOLS` selects the release-tools checkout; its exact pin is checked.

## Dependencies

Reference environment: Ubuntu 24.04-compatible host, Python 3.12, GCC/G++ 13,
Pillow 10, binutils, ARM64 GNU cross compiler/runtime and qemu-user. For an
Ubuntu 24.04 development machine, these packages provide the required tools:

```sh
sudo apt install build-essential python3 python3-pil g++-13-aarch64-linux-gnu binutils-aarch64-linux-gnu qemu-user
```

The reference cross compiler is GCC 13.3 with an ARM64 glibc 2.39 sysroot.
C++ and GCC runtimes are linked statically. The packager rejects non-AArch64
binaries, additional public symbols and glibc requirements above 2.39.
Only VSTPluginMain is exported. Using a newer cross sysroot can produce a
binary unsuitable for the MPC even if it compiles successfully.

## Build, test, package

```sh
python3 tools/build.py all
python3 tools/test.py > build/test.log 2>&1
python3 tools/test_arm64.py > build/arm-test.log 2>&1
python3 tools/package.py
```

`all` builds native and ARM64 libraries and the complete skin. `test.py` builds
native ASan/UBSan binaries and checks the real VST2 wrapper. `test_arm64.py`
loads the actual ARM64 .so under QEMU. These offline checks do not measure device
CPU. Package only after both suites pass.

The packager uses OpenMPC release.py, validates the portable catalog format,
archive integrity and every checksum, then writes the installation-only ZIP to
`build/releases/` by default. Desktop copies require the explicit `--desktop`
option. Source, screenshots,
compilers, test executables and debug logs are excluded from the installer.
It does not install, restart MPC, or publish a GitHub release. Packaging uses
`--catalog --require-portable --expect-id --expect-repo`. `--no-desktop` explicitly selects the default artifact location. A device benchmark can be attached with
`--bench benchmarks/0.7.4/bench-standard.txt` only when its binary SHA-256 matches exactly;
FAIL results are rejected. A rebuilt binary needs a fresh measurement instead
of silently inheriting a previous build’s CPU result.

`MPC_VST` selects another toolchain checkout. `CC` / `CXX` select native compilers;
`AARCH64_CC` / `AARCH64_CXX` select cross compilers. Installed tools are used when
available. A developer may alternatively unpack the toolchain below
`.tools/toolchain/usr` and QEMU below `.tools/qemu/usr`; these local files are
excluded from Git. QEMU tries the local ARM64 runtime, then
`/usr/aarch64-linux-gnu`; `QEMU_LD_PREFIX` explicitly selects another runtime.

## Artwork and documentation

`browser_tile.py` draws the 270x110 Sounds tile before gen_vst.py. After skin
generation, finish_skin.py draws the native page backgrounds, 128-frame 92x92 rotary strips,
switches, selectors and step outlines at 3x resolution. group_mod_pages.py computes
Q-Link frames from complete native control bounds, including names and displays.
The generated skin folder is cleaned before rebuilding, to avoid stale assets.

browser_preset.py exports a real Init chunk from the native VST2 build, verifies
its JUCE bank/base64/UID and restores it over a wet preset before writing
Presets/0000-Init.xpl. Run `all` on a clean checkout before building just `skin`.

```sh
python3 tools/build.py skin
python3 tools/render_readme.py
```

`render_readme.py` produces offline previews and overwrites the screenshot names.
The published README/gallery now use reviewed real Live III captures instead;
do not replace those with offline renders when preparing a release.

## Performance

The inference translation unit alone uses -O3 and ARM64 Cortex-A76 scheduling;
other DSP keeps its previous flags, and sanitizers use -O1. No fast-math,
downsampling, smaller network or silence gate is used. The fixed 44.1 kHz path
is checked sample for sample against LinInterp(1) for all 37 models.

[Performance evidence](docs/PERFORMANCE.md) documents the isolated real-device measurements.
`tools/model_bench.c` reuses the pinned OpenMPC benchmark host/timer. It loads a
private .so copy and never attaches to MPC or installs anything. Device access,
temporary-file writes and any service operations require the owner's separate
authorization; the build/test commands above are entirely local.

For an unpublished revised skin candidate, `--desktop-name` selects a distinct
ZIP basename without overwriting an earlier candidate. The manifest version stays
fixed until publication; published release assets must never be replaced.
The current [OpenMPC release guide](https://github.com/open-mpc/mpc-vst-plugins/blob/main/docs/RELEASING.md) describes publication and catalog requirements.

model_overlay.py converts the model chooser into a known native full-screen
CustomComponent overlay using KnobOverlay focus dismissal. Its visual backdrop
is backgroundData, not a full-screen Image input target. Page Q-Link rectangles
stay unchanged, and parameter 65 remains reserved for compatibility.

## 0.3.0 synthesis update

The new DSP is pinned to JC303 v0.14.0 (b0ff26011f80e45c7796cad7b29fbc7f347b9337).
Both source trees compile, with distinct namespaces and object filenames; only
the selected core renders audio. The original legacy sources and neural-network
optimizations remain intact. ModernVoice.cpp isolates modern headers behind an
opaque interface. JC303-MPC-5 stores 20 appended engine controls; versions 1–4
restore Legacy. Old VST parameter indices 0–65 remain unchanged, including the
previous implicit driveModel__open flag, now declared explicitly before new keys.

Distribution is a GPLv3/AGPLv3 combination. Read docs/LICENSES.md and retain both
license texts plus dependency notices. Public release artifacts must reference
matching published source. Package output defaults to build/releases; --desktop
is an explicit opt-in. Offline tests are not MPC CPU measurements.


0.7.0 navigation is defined by tools/navigation.py and applied after native
controls/Drive overlay generation by tools/group_mod_pages.py. It rebuilds
RowTopDown screen banks from visible knobs, with Drive Model as the sole
selector exception. Final focus rectangles use the actual placed native control
bounds and popup heading. The SYNTH layout qlinks seed deliberately preserves
the published CC20–35 table; it does not define the final screen banks. Existing
wrapper ABI and async pins are unchanged. The additional engine-program-bank
patch expands the native menu independently of historical preset automation. Do not remove that compatibility seed when
changing the screen layout. tools/test_sequencer_skin.py verifies both bindings
and complete label/value containment, in addition to the 176 flag-cell tests.

## Original factory bank (1.0.0 candidate)

`tools/factory_presets.py` compiles the 18 pinned upstream JSON sounds into
immutable numeric tables. The engine exposes 30 VST programs through the
engine-program-bank wrapper patch; parameter 64 keeps its original twelve
choices and normalized range. No parameter identities were added or shifted.
The JSON source, attribution and conversion rules live under `factory/` and
`tools/factory_presets.py`. `tools/browser_preset.py` exports all 30 real XPL
states with matching UID, version and VST bank count and verifies mapped
values and full roundtrips. Factory-load audio, sequence/tuning retention,
edited project recall and host echoes are exercised by `tests/factory_checks.h`
in both sanitizer and actual ARM64 VST tests. Older versions reject states
selecting the new factory program indices; keep original project backups.
