# Manufacturer update — 1.0.0

Manufacturer: `303inmyheart`. Catalog name, plugin UID, `.so` filename,
parameter identities/ranges and the JC10 state layout remain unchanged.
The VST program bank expands from 12 to 30 entries independently of the old
12-choice preset automation parameter. Historical engine chunks remain
readable; MPC project reopening across the folder change still needs the
device check. New states selecting factory program 12–29 require this version
or newer. Original engine authors
remain credited in the README and licence notices.

The OpenMPC Installer package declares `midilab - VST - JC303 MPC` in `retire` and installs into
`303inmyheart - VST - JC303 MPC`. The old folder is removed only when its manifest carries this
plugin's catalog id.
Normal MPC user presets/projects outside the installed folder are untouched.
Use an installer that processes `retire`; copying the new folder by hand leaves
the previous copy behind. If you manually added other files inside the old
plugin folder, back those up before migration: retirement keeps declared user
settings, not arbitrary files in the former installation directory.

All 18 original v0.14 factory sounds are included in the native VST program
menu and exported as real MPC XPL states. The existing 12 sounds retain their
indices and names. Sound switching keeps the current pattern/tuning. Full MPC
XPL loading restores its saved sequence/tuning like any complete native preset.
MODS master and per-area switches are enabled for sounds that use them; dry
Init defaults remain Off. Original macro assignments do not alter Q-Links.
See [factory/NOTICE.md](factory/NOTICE.md) for source and licence attribution.

Native/ARM, package and
isolated installation tests are recorded separately. Installation and folder migration are complete. Project reopen checks and a
fresh exact-binary benchmark remain pending. Prior release
measurements are retained as historical results and are not attached to this
new binary. The update was installed on Live III on 2026-11-27 after explicit approval.
Package and installed-file hashes, registry migration and application restart
were verified; user interface, sound and old-project acceptance remain pending.

## Completed local checks

Native ASan/UBSan and actual ARM64/QEMU suites passed, including all 18
factory programs, unchanged historical preset automation, sound rendering,
sequence/tuning preservation, edited-state recall and host echo safety.
All 30 exported XPL states pass parameter and complete-state roundtrips and
carry the matching 30-program VST bank count. Actual ARM64 vendor/UID/version
are 303inmyheart / 4a634d33 / 10000. Current catalog validation: zero warnings.
Old-to-new install, repeat install, native preset paths, uninstall and foreign-id
safety pass in an isolated local fixture. Package checksum/privacy checks pass.
Final exact-binary benchmarks are complete: standard WARN 34.4% p99 /
37.5% max, full stress WARN 33.9% / 38.2%. Broader user acceptance remains
separate from these measurements.

## Historical MPC preset headers — compatibility finding

MPC may reject an old user XPL before passing its state to the instrument when
its PLUGIN manufacturer is still `midilab`. Keeping the VST UID alone does not
establish host acceptance across this manufacturer change. A historical JC3
chunk has been verified to load and migrate in the actual current native VST;
its old header still needs a compatible copy with manufacturer `303inmyheart`
and the current installed plugin path. Preserve the original and the encoded
state verbatim. Do not bulk-edit user files or embed user sounds in releases.
One such copy was created on Live III after explicit user approval; loading through the native MPC browser was confirmed by the maintainer. Historical
project reopening and a general opt-in migration workflow remain release checks.
