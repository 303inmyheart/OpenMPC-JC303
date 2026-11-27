# Read before updating: JC303 MPC 1.0.0

**The manufacturer changes from `midilab` to `303inmyheart`. Older personal MPC presets
can need migration even though the plugin UID stays the same. The installer
does not migrate user preset files automatically.**

## What users may see

The MPC can reject an older `.xpl` with “This state file is not for JC303 MPC”
before passing its sound data to the synth. Do not overwrite that preset with
Init: the rejection does not establish that its sound data are damaged.

## Easiest option: the sound is saved inside an old project

You may be able to create a compatible preset using only the MPC:

1. Keep backups of your original project and presets.
2. Open the older project and select its JC303 track.
3. Play the sound and check its settings. Continue only if it sounds correct.
4. Use MPC's normal plugin-preset Save command. Save under a **new name**;
   do not replace the original preset or save Init over it.
5. Load that new preset in a fresh JC303 instance and check the sound again.
   Keep the original files as backups.

This route does not require editing XML. The maintainer confirmed that JC303
loads in the prepared old-project test; saving and reloading a new preset from
that project is an additional check, not yet a claimed hardware test.

If you have only the older `.xpl` file, continue below. `.xpl` is the small file
MPC writes when saving a plugin preset. Converting its metadata is a file task
on a computer, not a synthesizer setting or a Q-Link control. Ask for help if
you are not comfortable editing it; keep the original and encoded sound data.

## Recommended: the Python converter

Use the [common preset converter](docs/PRESET-CONVERTER.md) for both JC303 MPC
and Geonkick MPC. It previews the changes, creates separate compatible copies
and leaves your original files and encoded sounds unchanged. It runs on your
computer; the plugin installer does not convert personal presets automatically.
Manual XML editing below is an alternative, not a requirement.

## Keep the original; migrate a copy

1. Before updating, back up your own presets and projects outside the installed
   plugin folder. Keep the original files unchanged.
2. If a preset is rejected, prepare a separate copy of its `.xpl`. In its XML
   `PLUGIN` metadata, change `manufacturer="midilab"` to
   `manufacturer="303inmyheart"` and set `file` to the actual installed `.so` path:
   `<drive>/VST2 Plugins/303inmyheart - VST - JC303 MPC/jc303_mpc.so`. Replace `<drive>` with the drive
   containing the installed plugin; do not use that placeholder literally.
3. Preserve `uid="4a634d33"` and the entire encoded `state` element exactly.
   Do not change the state payload, the preset's other sound data or UID.
4. Save the copy in your normal MPC user preset storage under
   `MPC Documents/Plugin Presets/Instruments/303inmyheart - VST - JC303 MPC/`.
   Use a different filename if one already exists. Never overwrite an existing
   preset. Reopen the native preset list and load the compatible copy.
5. Check the sound before saving further edits. Keep your original backup.

The separate Python converter handles the supported XPL header changes. The
plugin installer only migrates installed plugin folders/settings; it does not
run the converter. Keep originals and check the copied sounds on the MPC.

## Validation scope

A historical JC3 sound chunk was accepted and migrated by the actual current
VST. The maintainer then confirmed loading the header-corrected copy through
the native MPC browser on Live III. Original file and encoded sound data were
verified unchanged. This is one confirmed migration, not an all-presets claim.

Existing project reopening across the manufacturer change is a separate check;
no blanket project-compatibility claim is made.

## In-plugin help

PRESET INFO in the black header opens the English guide.
[Rendered guide preview](https://github.com/303inmyheart/OpenMPC-JC303/blob/jc303-mpc-v1.0.0/docs/preset-help.png) (local render, not a device screenshot).
