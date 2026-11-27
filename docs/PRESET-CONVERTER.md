# Preset converter — JC303 MPC and Geonkick MPC

The same small Python program supports both MPC ports. It updates the old
manufacturer and installed plugin path in **copies** of `.xpl` presets.
Original files, preset names, plugin UIDs and encoded sound data stay intact.
It works on your computer, with no internet connection or SSH access.

## Download and start

[Download/view the Python script](../tools/convert_mpc_presets.py). On GitHub,
use **Download raw file** and save it as `convert_mpc_presets.py`.
Python 3.8 or newer is required; no pip packages are needed. Open a terminal in
the folder where you saved it and run:

```sh
python3 convert_mpc_presets.py
```

A folder picker opens if Python's optional Tk component is available. Otherwise,
the terminal wizard asks for the same information; nothing needs installing.
On Windows, `py -3 convert_mpc_presets.py` can be used instead.

1. Choose the folder containing your older `.xpl` presets. Subfolders are read
   too. Choose a separate output folder for the new copies.
2. Check the **plugin location on the MPC**. The default `/nvme/VST2 Plugins`
   is for a Live III with plugins installed there. Use the actual MPC path for
   another drive/model; a Windows/macOS disk path is not the correct value.
3. Preview the result. Unrelated plugins and already-current files are skipped;
   malformed files are reported rather than guessed. No copies are written yet.
4. Confirm **Create copies**, or type `CONVERT` in the terminal wizard.
5. Copy the generated `303inmyheart - VST - ...` folders to
   `MPC Documents/Plugin Presets/Instruments/` on the MPC. Preserve your existing
   files when copying. Reopen its preset list and try a few converted sounds.

The program does not send files to the MPC automatically. Keep your original
backups until the sounds have been checked. Do not replace originals with Init.

## Command-line preview and conversion

```sh
python3 convert_mpc_presets.py --source "Old Presets" --output "Converted Presets"
python3 convert_mpc_presets.py --source "Old Presets" --output "Converted Presets" --apply
```

The first command only previews. The second explicitly creates copies. Existing
output files are never overwritten: identical copies are reused, and different
files get a numbered filename. Input changes after preview are detected. Symbolic
links are not followed. Private output defaults outside a repository checkout.

## Limits and verification

Only the known JC303 MPC / midilab and Geonkick MPC / OpenMPC identities are
supported, plus their current 303inmyheart identity when a path needs updating.
It does not rewrite MPC projects, `.gkick`/`.gkit` files or synthesis parameters,
and cannot repair damaged sound data. Compatible-header conversion does not
establish that every old preset will load/play correctly; check on the MPC.

Automated checks cover both plugin identities, unchanged content outside the
header, BOM/line endings, UID/state preservation, malformed/unrelated files,
no-overwrite/idempotence, input changes and symlink/output-boundary protection.
A 131-preset factory test batch and deliberate negative fixtures also passed.
The GUI itself was not runtime-tested where optional Tk is unavailable.
