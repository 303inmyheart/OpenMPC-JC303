# Changelog

## 1.0.0 — 2026-11-27

> **PRESET COMPATIBILITY NOTICE:** IMPORTANT: Manufacturer changed from midilab to 303inmyheart. Older personal MPC presets may be rejected as "not for this plugin". Keep originals and use migrated copies; updating the plugin does not convert your user presets.

Adds all 18 original JC303 v0.14 factory sounds: 30 native MPC programs in total. Existing parameter identities and the twelve-choice legacy preset automation range remain unchanged.

A visible **PRESET INFO** button in the black header opens English
instructions on every native page. The expanded plain-language guide explains
backups, resaving a sound from an older project without file editing, and the
separate file-conversion route for users who only have an old XPL. It is information, not an automatic converter.
It has no Q-Link assignment. Hardware interaction acceptance is pending.

The manufacturer change affects MPC preset metadata, not just the displayed
folder name. Keeping the plugin UID alone does not guarantee host acceptance.
User presets are not automatically converted by the standard installer.
Read [Preset migration](PRESET-MIGRATION.md) **before updating**.

A compatible copy of a historical JC3 preset was loaded successfully by the
maintainer on Live III. Its original and encoded sound data were unchanged.
This does not establish compatibility of every historical preset or project.

Final exact-binary benchmarks: standard WARN
34.4% p99 / 37.5% max; additional full stress WARN 33.9% / 38.2%.
