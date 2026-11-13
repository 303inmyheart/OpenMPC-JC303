# Build OpenMPC JC303

The supported target is MPC Live III / ARM64, MPC OS 3.10.1 or newer.
DSP/model dependencies are vendored; no JUCE checkout, desktop GUI or Steinberg
SDK is needed for this port. Builds use a separate, pinned OpenMPC toolchain.

## Checkout

```sh
git clone --branch mpc-live-iii https://github.com/303inmyheart/OpenMPC-JC303.git
cd OpenMPC-JC303
git clone https://github.com/open-mpc/mpc-vst-plugins.git ../mpc-vst-plugins
git -C ../mpc-vst-plugins checkout ef05a26c6e59f1bfd3ef2023e6a9d8b371b61310
git -C ../mpc-vst-plugins apply "$PWD/patches/mpc-float32.patch"
```

Use that commit and apply the included patch once to a clean checkout. The
patch supplies the float engine ABI, stepped Q-Link feedback and host loop
metadata this port expects. It includes the new audio_format.h file.

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
`~/Schreibtisch`. It refuses to overwrite an existing ZIP. Source, screenshots,
compilers, test executables and debug logs are excluded from the installer.
It does not install, restart MPC, or publish a GitHub release.

`MPC_VST` selects another toolchain checkout. `CC` / `CXX` select native compilers;
`AARCH64_CC` / `AARCH64_CXX` select cross compilers. Installed tools are used when
available. A developer may alternatively unpack the toolchain below
`.tools/toolchain/usr` and QEMU below `.tools/qemu/usr`; these local files are
excluded from Git. QEMU tries the local ARM64 runtime, then
`/usr/aarch64-linux-gnu`; `QEMU_LD_PREFIX` explicitly selects another runtime.

## Artwork and documentation

`browser_tile.py` draws the 270x110 Sounds tile before gen_vst.py. After skin
generation, finish_skin.py draws the six panels, 128-frame 92x92 rotary strips,
switches, selectors and step outlines at 3x resolution. skin_focus.py computes
Q-Link frames from complete native control bounds, including names and displays.
The generated skin folder is cleaned before rebuilding, to avoid stale assets.

browser_preset.py exports a real Init chunk from the native VST2 build, verifies
its JUCE bank/base64/UID and restores it over a wet preset before writing
Presets/0000-Init.xpl. Run `all` on a clean checkout before building just `skin`.

```sh
python3 tools/build.py skin
python3 tools/render_readme.py
```

The README previews query the built plugin for displayed values. They approximate
MPC host fonts with DejaVu and do not claim to be hardware screenshots.

## Performance

The inference translation unit alone uses -O3 and ARM64 Cortex-A76 scheduling;
other DSP keeps its previous flags, and sanitizers use -O1. No fast-math,
downsampling, smaller network or silence gate is used. The fixed 44.1 kHz path
is checked sample for sample against LinInterp(1) for all 37 models.

[PERFORMANCE.md](PERFORMANCE.md) documents the isolated real-device measurements.
`tools/model_bench.c` reuses the pinned OpenMPC benchmark host/timer. It loads a
private .so copy and never attaches to MPC or installs anything. Device access,
temporary-file writes and any service operations require the owner's separate
authorization; the build/test commands above are entirely local.
