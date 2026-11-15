# JC303 MPC catalog and release status

Target: Gen2 MPCs (aarch64), MPC OS 3.10.1+. Hardware evidence: Live III only.
Catalog id `jc303-mpc`, display name **JC303**, UID `JcM3` and installed name/folder
**JC303 MPC** remain unchanged. Repo: 303inmyheart/OpenMPC-JC303.

## Release 0.7.4

- Stable GitHub release with local ARM64 build and matching adapted source tag.
- Portable OpenMPC installer scripts, manifests, checksums, Init, Sounds tile,
  native skin and full license notices.
- Combined license `GPL-3.0-only AND AGPL-3.0-or-later`, retained upstream notices.
- Native sanitizer/full ARM64/QEMU/UI/geometry and catalog/package checks pass.
- Exact real Live III standard CPU **WARN, p99 34.6% / max 38.3%**, attached to installer;
  additional full 348-parameter stress **WARN, p99 33.5% / max 43.0%**, separately retained.
- Root tested.json records exact hardware workload and its limits; current SETUP
  screenshot confirms Off defaults, not a claim that every new control was
  manually retested. No untested Gen2 model is claimed.
- Eighteen real hardware screenshots; six selected in README, full gallery linked.

See [final evidence](benchmarks/0.7.4/README.md), [TESTING.md](TESTING.md),
[license audit](LICENSE-AUDIT.md) and [build instructions](BUILDING.md).

## Existing catalog entry

[Initial PR845](https://github.com/open-mpc/mpc-vst-plugins/pull/845) was merged.
The entry needs a metadata update for the 64-step feature set, combined license and
new release-tag-pinned screenshot. No new id, manual version or checksum belongs
in that JSON. Subsequent valid stable GitHub releases are discovered by the
nightly catalog build; uploading a ZIP into the repository alone is insufficient.

The [official current contribution guide](https://github.com/open-mpc/mpc-vst-plugins/blob/main/catalog/README.md)
describes release discovery and tested.json. Runtime wrapper and release tooling
remain pinned in BUILDING.md/release.json; current registry validation accepts
compound open-source SPDX expressions with AND/OR.

## Update behavior

Install the new intact ZIP over the existing JC303 installation, save project
copies first, then restart the MPC application. The identity is retained; old
saved states migrate with their flags. Older plugin versions cannot read JC10
states written by 0.7.4. Personal projects, settings and unrelated plugins are not
part of the release. The release library is identical to the installed 0.7.4 one;
adding benchmark metadata did not rebuild or change its audio.
