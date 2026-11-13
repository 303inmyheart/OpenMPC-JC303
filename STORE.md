# OpenMPC catalog / release readiness

Target: Gen2 MPCs (aarch64), MPC OS 3.10.1+. Hardware evidence: MPC Live III only.
Catalog ID `jc303-mpc`, UID `JcM3` (`4a634d33`) and installed filename
`jc303_mpc.so` are unchanged.

## Rules reviewed

Review based on OpenMPC commit `a2dd2b78a19c4cd46257c09eb9a485a155ab2968`:

- [Catalog contribution guide](https://github.com/open-mpc/mpc-vst-plugins/blob/a2dd2b78a19c4cd46257c09eb9a485a155ab2968/catalog/README.md)
- [Release procedure](https://github.com/open-mpc/mpc-vst-plugins/blob/a2dd2b78a19c4cd46257c09eb9a485a155ab2968/docs/RELEASING.md)
- [Catalog schema](https://github.com/open-mpc/mpc-vst-plugins/blob/a2dd2b78a19c4cd46257c09eb9a485a155ab2968/docs/CATALOG_SPEC.md)
- [Benchmark procedure](https://github.com/open-mpc/mpc-vst-plugins/blob/a2dd2b78a19c4cd46257c09eb9a485a155ab2968/docs/BENCH.md)
- [Quality / tiers](https://github.com/open-mpc/mpc-vst-plugins/blob/a2dd2b78a19c4cd46257c09eb9a485a155ab2968/docs/CATALOG_QUALITY.md)
- [Workflow guide](https://github.com/open-mpc/mpc-vst-plugins/blob/a2dd2b78a19c4cd46257c09eb9a485a155ab2968/catalog/pages/workflow.md)

Some schema prose still describes Gen1 only; the actual current checker accepts
Gen2 AArch64 and glibc up to 2.39. Both manifest and registry are checked using
that code, with pinned release templates.

## Prepared

- Public fork, source, GPL-3.0-only license, upstream/dependency credits and build instructions.
- Current portable installer format, `plugin.json` with `file64`, browser tile and real Init preset.
- `source_repo`, license, Gen2 requirements and official CPU result in the manifest.
- Native sanitizer and actual ARM64 shared-object checks under QEMU.
- Official Live III CPU benchmark: **WARN**, p99 **16.5%**, max **24.6%** of one core.
- Neutral visible drive-model labels; original model identities/credits in MODELS.md.
- Catalog entry: [catalog/plugins/jc303-mpc.json](catalog/plugins/jc303-mpc.json).
- Commit-pinned public screenshot URL and Gen2-only asset pattern.
- Root [tested.json](tested.json) truthfully records installed 0.2.2 on Live III / OS 3.10.1.
- Build/validation workflow that packages an artifact for device testing. Rebuilt artifacts do not silently inherit another binary's CPU measurement.

Local validation:

```sh
python3 ../mpc-vst-plugins-release-tools/tools/catalog_build.py --registry catalog/plugins --check-registry
python3 ../mpc-vst-plugins-release-tools/tools/catalog_check.py build/releases/JC303-MPC-0.2.3-mpc-aarch64.zip --catalog --require-portable --expect-id jc303-mpc --expect-repo 303inmyheart/OpenMPC-JC303
```

Both pass. The installer validation reports zero warnings. This is distinct from
the CPU **WARN**, which is disclosed in release notes as the OpenMPC procedure requires.

## Confirmed release artifact

The maintainer confirmed Candidate 5 of 0.2.3 on MPC Live III / MPC OS 3.10.1,
including playback with and without amp, native model-overlay layering and
outside-tap dismissal, and explicitly authorized submission. Root tested.json
records this exact version separately from 0.2.2. Candidates 1–4 are superseded.

The published archive is byte-identical to the confirmed Candidate 5 desktop ZIP.
Its SHA-256 is in the release's SHA256SUMS asset. No other Gen2 model is claimed
as hardware-tested. No device installation or service restart was performed by
the release-preparation tools.

## Submit to the catalog

The catalog submission is a PR to open-mpc/mpc-vst-plugins adding only
`catalog/plugins/jc303-mpc.json`, copied from this repository. No versions or
checksums go in that registry file: they are discovered from GitHub releases.
The catalog CI must validate the published release; drafts are not discoverable.
Merging is up to OpenMPC's maintainers. Later releases are discovered nightly.
See the GitHub pull requests in open-mpc/mpc-vst-plugins for the submission and maintainer review.
