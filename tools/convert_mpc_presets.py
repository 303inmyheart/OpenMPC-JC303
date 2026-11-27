#!/usr/bin/env python3
"""Copy and update legacy JC303/Geonkick MPC preset metadata; never edit originals.

SPDX-License-Identifier: GPL-3.0-or-later
Python 3.8+, standard library only. No network access, SSH or device control.
No arguments: optional Tk file picker, otherwise a simple terminal wizard.
CLI: --source DIR --output DIR [--plugin-root PATH] [--apply]
Without --apply, the CLI only previews what would change.
"""
import argparse
from dataclasses import dataclass
import os
from pathlib import Path, PurePosixPath
import re
import sys
from typing import Optional
import xml.etree.ElementTree as ET
from xml.sax.saxutils import escape

DEFAULT_PLUGIN_ROOT = '/nvme/VST2 Plugins'  # Live III; configurable for other drives
MAX_BYTES = 64 * 1024 * 1024
PLUGINS = {
    '4a634d33': ('JC303 MPC', 'midilab', 'jc303_mpc.so'),
    '476b4d33': ('Geonkick MPC', 'OpenMPC', 'geonkick_mpc.so'),
}
TAG = re.compile(r'''<PLUGIN\b(?:[^>"']|"[^"]*"|'[^']*')*>''')
ATTR = re.compile(r'''([\w:.-]+)\s*=\s*(?:"([^"]*)"|'([^']*)')''')


class GuiUnavailable(RuntimeError):
    pass


@dataclass
class Result:
    source: Path
    status: str
    message: str
    folder: str = ''
    data: Optional[bytes] = None
    destination: Optional[Path] = None
    output_root: Optional[Path] = None


def plugin_root(value):
    if not value.startswith('/') or '\\' in value or any(c in value for c in '\n\r\x00'):
        raise ValueError('Use the absolute MPC path, for example /nvme/VST2 Plugins, not a Windows path.')
    path = PurePosixPath(value)
    if '..' in path.parts or path.name != 'VST2 Plugins':
        raise ValueError('The MPC plugin location must end with VST2 Plugins and contain no .. components.')
    return str(path)


def inspect(source, location=DEFAULT_PLUGIN_ROOT):
    """Return converted bytes in memory only. Everything outside PLUGIN stays intact."""
    source = Path(source)
    try:
        location = plugin_root(location)
        if source.is_symlink():
            return Result(source, 'skipped', 'Symbolic links are not followed.')
        if source.stat().st_size > MAX_BYTES:
            raise ValueError('File exceeds the 64 MiB safety limit.')
        raw = source.read_bytes()
        text = raw.decode('utf-8-sig')
        if re.search(r'<!\s*(DOCTYPE|ENTITY)\b', text, re.I):
            raise ValueError('XML document types/entities are not supported.')
        tree = ET.fromstring(text)
        if tree.tag != 'pluginstate':
            return Result(source, 'skipped', 'Not an MPC plugin preset.')
        descriptions = tree.findall('PLUGIN')
        if len(descriptions) != 1:
            raise ValueError('Expected exactly one PLUGIN description.')
        description = descriptions[0]
        uid = description.get('uid', '').lower()
        if uid.startswith('0x'):
            uid = uid[2:]
        if uid not in PLUGINS:
            return Result(source, 'skipped', 'Another plugin: left unchanged.')
        name, old_vendor, library = PLUGINS[uid]
        if description.get('name') != name:
            return Result(source, 'skipped', 'Plugin name and UID do not match a supported MPC port.')
        if description.get('manufacturer') not in (old_vendor, '303inmyheart'):
            return Result(source, 'skipped', 'Unexpected manufacturer: left unchanged.')
        state = tree.findall('state')
        if len(state) != 1 or not (state[0].text or '').strip():
            raise ValueError('Preset has no complete encoded sound state.')
        folder = '303inmyheart - VST - ' + name
        filename = str(PurePosixPath(location) / folder / library)
        if description.get('manufacturer') == '303inmyheart' and description.get('file') == filename:
            return Result(source, 'current', 'Already uses the selected manufacturer and plugin path.', folder)
        tags = list(TAG.finditer(text))
        if len(tags) != 1:
            raise ValueError('Ambiguous PLUGIN XML; refusing to guess.')
        tag = tags[0]
        changes = []
        for field, value in [('manufacturer', '303inmyheart'), ('file', filename)]:
            matches = [m for m in ATTR.finditer(tag.group()) if m.group(1) == field]
            if len(matches) != 1:
                raise ValueError('Missing or ambiguous ' + field + ' attribute.')
            match = matches[0]
            group = 2 if match.group(2) is not None else 3
            changes.append((match.start(group), match.end(group), escape(value, {'"': '&quot;', "'": '&apos;'})))
        updated = tag.group()
        for start, end, replacement in sorted(changes, reverse=True):
            updated = updated[:start] + replacement + updated[end:]
        result_text = text[:tag.start()] + updated + text[tag.end():]
        # No state reconstruction, decoding, normalization or parameter remapping.
        data = (b'\xef\xbb\xbf' if raw.startswith(b'\xef\xbb\xbf') else b'') + result_text.encode('utf-8')
        checked = ET.fromstring(data)
        if ET.tostring(checked.find('state')) != ET.tostring(state[0]):
            raise ValueError('Sound-state integrity check failed.')
        if checked.find('PLUGIN').get('uid') != description.get('uid'):
            raise ValueError('Plugin UID integrity check failed.')
        return Result(source, 'ready', name + ': maker name and installed path will be updated.', folder, data)
    except (OSError, ValueError, UnicodeError, ET.ParseError) as error:
        return Result(source, 'error', str(error))


def files_in(source, output):
    """Snapshot the input; do not traverse symlinks or our own output directory."""
    source, output = Path(source).absolute(), Path(output).absolute()
    if source.is_symlink():
        raise ValueError('Choose a real file/folder, not a symbolic link.')
    if source.is_file():
        if source.suffix.lower() != '.xpl':
            raise ValueError('Select an MPC .xpl preset or a folder containing presets.')
        return source.parent, [source]
    if not source.is_dir():
        raise ValueError('Input folder does not exist.')
    if source.resolve() == output.resolve():
        raise ValueError('Input and output folders must be different.')
    result = []
    for directory, dirs, files in os.walk(source, followlinks=False):
        dirs[:] = sorted(d for d in dirs if not (Path(directory)/d).is_symlink()
                         and (Path(directory)/d).resolve() != output.resolve())
        result.extend(Path(directory)/name for name in sorted(files) if Path(name).suffix.lower() == '.xpl')
    return source, result


def plan(source, output, location=DEFAULT_PLUGIN_ROOT):
    location = plugin_root(location)
    base, paths = files_in(source, output)
    results = []
    known_folders = {vendor + ' - VST - ' + name for name, vendor, _ in PLUGINS.values()}
    known_folders |= {'303inmyheart - VST - ' + name for name, _, _ in PLUGINS.values()}
    for path in paths:
        item = inspect(path, location)
        relative = path.relative_to(base)
        if relative.parts[0] in known_folders:
            relative = Path(*relative.parts[1:])
        if item.status == 'ready':
            item.destination = Path(output).absolute() / item.folder / relative
            item.output_root = Path(output).resolve()
        results.append(item)
    return results


def write_copy(item):
    """Exclusive creation: never overwrite an original or any existing output."""
    assert item.status == 'ready' and item.destination and item.data
    assert item.output_root
    try:
        item.destination.parent.resolve().relative_to(item.output_root)
    except ValueError:
        raise ValueError('An output subfolder points outside the selected output folder. No copy was written.')
    item.destination.parent.mkdir(parents=True, exist_ok=True)
    destination = item.destination
    for count in range(10000):
        candidate = destination if count == 0 else destination.with_name(destination.stem + ' (converted %d)' % count + destination.suffix)
        if candidate.is_symlink():
            continue
        if candidate.exists():
            if candidate.is_file() and candidate.stat().st_size == len(item.data) and candidate.read_bytes() == item.data:
                return 'existing', candidate
            continue
        try:
            with candidate.open('xb') as output:
                try:
                    output.write(item.data)
                    output.flush()
                    os.fsync(output.fileno())
                except OSError:
                    # Only remove the failed file created by this open handle.
                    if candidate.stat().st_ino == os.fstat(output.fileno()).st_ino:
                        candidate.unlink()
                    raise
            if candidate.read_bytes() != item.data:
                raise OSError('Written copy did not match the verified conversion.')
            return 'created', candidate
        except FileExistsError:
            continue
    raise OSError('Too many existing filenames; no file was overwritten.')


def apply(results, location):
    counts = {'created': 0, 'existing': 0, 'current': 0, 'skipped': 0, 'error': 0}
    for item in results:
        if item.status != 'ready':
            counts[item.status] += 1
            continue
        # Re-read after preview: do not convert a file changed in the meantime.
        fresh = inspect(item.source, location)
        if fresh.status != 'ready' or fresh.data != item.data:
            counts['error'] += 1
            continue
        status, _ = write_copy(item)
        counts[status] += 1
    return counts


def summary(results):
    counts = {key: sum(item.status == key for item in results) for key in ['ready', 'current', 'skipped', 'error']}
    return ('Ready to convert: {ready}\nAlready current: {current}\nOther/skipped: {skipped}\nInvalid files: {error}').format(**counts)


def result_summary(counts):
    return ('New copies created: {created}\nIdentical copies already present: {existing}\n'
            'Already current: {current}\nOther/skipped: {skipped}\nErrors: {error}\n'
            'Original preset files were not edited.').format(**counts)


def default_folders():
    here = Path(__file__).resolve().parent
    bundled = here/'Test Presets'/'Valid'
    if bundled.is_dir():
        return str(bundled), str(here/'Converted Presets')
    # A repository download has no test bundle. Ask for the input explicitly
    # and keep private output outside the source repository by default.
    return '', str(Path.home()/'Converted MPC Presets')


def wizard():
    print('\nOpenMPC Preset Converter - JC303 MPC and Geonkick MPC\n')
    print('No originals are edited. Only new .xpl copies are created. No network connection.\n')
    default_source, default_output = default_folders()
    source = input('Input folder [%s]: ' % default_source if default_source else 'Input folder: ').strip() or default_source
    if not source:
        raise ValueError('Choose an input folder containing your .xpl presets.')
    output = input('Output folder [%s]: ' % default_output).strip() or default_output
    print('The next path is ON YOUR MPC, not a Windows/macOS path.')
    location = plugin_root(input('MPC plugin location - Live III default [%s]: ' % DEFAULT_PLUGIN_ROOT).strip() or DEFAULT_PLUGIN_ROOT)
    results = plan(source, output, location)
    print('\n'+summary(results)+'\n\nMPC plugin location: '+location)
    for item in results:
        if item.status == 'error':
            print('Invalid: '+str(item.source)+' - '+item.message)
    if not any(item.status == 'ready' for item in results):
        return
    if input('\nType CONVERT to create compatible copies (anything else cancels): ').strip() != 'CONVERT':
        print('Cancelled. No preset copies created.');return
    print(result_summary(apply(results, location)))
    print('Copies are in '+output+'\nCopy the plugin folders inside it to MPC Documents/Plugin Presets/Instruments on your MPC.')


def gui():
    import tkinter as tk
    from tkinter import filedialog, messagebox
    from tkinter.scrolledtext import ScrolledText
    try:window = tk.Tk()
    except tk.TclError as error:raise GuiUnavailable(str(error)) from error
    window.title('OpenMPC Preset Converter');window.geometry('830x570')
    initial_source, initial_output = default_folders()
    source = tk.StringVar(value=initial_source)
    output = tk.StringVar(value=initial_output)
    location = tk.StringVar(value=DEFAULT_PLUGIN_ROOT)
    tk.Label(window,text='JC303 MPC + Geonkick MPC',font=('',16,'bold')).pack(pady=10)
    tk.Label(window,text='Create compatible COPIES. Originals and encoded sound data remain unchanged.').pack()
    for label, variable, browse in [('Input folder',source,True),('Output folder',output,True),('Plugin location ON THE MPC (Live III default)',location,False)]:
        frame=tk.Frame(window);frame.pack(fill='x',padx=18,pady=6)
        tk.Label(frame,text=label).pack(anchor='w')
        tk.Entry(frame,textvariable=variable).pack(side='left',expand=True,fill='x')
        if browse:
            def choose(v=variable):
                path=filedialog.askdirectory()
                if path:v.set(path)
            tk.Button(frame,text='Browse...',command=choose).pack(side='right')
    log=ScrolledText(window,height=12,wrap='word');log.pack(fill='both',expand=True,padx=18,pady=6)
    state={}
    def preview():
        try:
            results=plan(source.get(),output.get(),location.get())
            state['results']=results;state['inputs']=(source.get(),output.get(),location.get())
            log.delete('1.0','end');log.insert('end',summary(results)+'\n\n')
            for item in results:log.insert('end',item.source.name+': '+item.status+' - '+item.message+'\n')
            convert.config(state='normal' if any(item.status=='ready' for item in results) else 'disabled')
        except (OSError,ValueError) as error:messagebox.showerror('Preview failed',str(error))
    def execute():
        if state.get('inputs')!=(source.get(),output.get(),location.get()):
            messagebox.showinfo('Preview needed','The folder/path changed. Preview again before converting.');return
        if not messagebox.askyesno('Create copies?', 'Create compatible copies in the output folder?\n\nOriginals will not be overwritten.'):return
        try:
            result=apply(state['results'],plugin_root(location.get()))
            log.insert('end','\n'+result_summary(result)+'\n\nCopy the generated plugin folders to MPC Documents/Plugin Presets/Instruments on your MPC.\n')
            convert.config(state='disabled')
        except (OSError,ValueError) as error:messagebox.showerror('Conversion failed',str(error))
    buttons=tk.Frame(window);buttons.pack(pady=10)
    tk.Button(buttons,text='1. Preview',command=preview).pack(side='left',padx=6)
    convert=tk.Button(buttons,text='2. Create copies',state='disabled',command=execute);convert.pack(side='left',padx=6)
    window.mainloop()


def main(argv=None):
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--source',type=Path)
    parser.add_argument('--output',type=Path)
    parser.add_argument('--plugin-root',default=DEFAULT_PLUGIN_ROOT)
    parser.add_argument('--apply',action='store_true',help='Create new copies after previewing with the same arguments.')
    parser.add_argument('--cli',action='store_true',help='Use the terminal wizard instead of the optional Tk interface.')
    args=parser.parse_args(argv)
    if args.source is None and args.output is None:
        if args.cli:wizard();return 0
        try:gui()
        except (ImportError,GuiUnavailable):
            print('The optional graphical interface is unavailable. Starting the terminal wizard.')
            wizard()
        return 0
    if args.source is None or args.output is None:
        parser.error('Supply both --source and --output, or run without arguments for the wizard.')
    location=plugin_root(args.plugin_root)
    results=plan(args.source,args.output,location)
    print(summary(results));print('MPC plugin location: '+location)
    for item in results:
        if item.status=='error':print('Invalid: '+str(item.source)+' - '+item.message)
    if args.apply:print(result_summary(apply(results,location)))
    else:print('Preview only. No files written. Add --apply to create copies.')
    return 1 if any(item.status=='error' for item in results) else 0


if __name__=='__main__':
    try:sys.exit(main())
    except (OSError,ValueError,EOFError) as error:
        print('Error: '+str(error),file=sys.stderr);sys.exit(2)
