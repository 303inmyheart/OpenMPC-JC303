# JC303 MPC 0.4.0 — 64-step sequencer candidate

Local development candidate; not a public release or store benchmark. Native
and ARM64/QEMU functional checks are recorded in TESTING.md after completion.
Touch, sound and host integration still require a real MPC acceptance test.

## Navigation and note input

SEQUENCE cycles through **SEQUENCE / ARTIC / SEQTOOLS / SEQLIVE / SEQKEY**; the six main
page keys and MODS / ENGINE / FILTER grouping remain. This is our own Acid
interface inspired by a piano-roll workflow, not a copy of Phoscyon 2.

- **STEP PAGE** selects 1–16, 17–32, 33–48 or 49–64. Grid column numbers are
  relative to the chosen page; the page selector supplies the absolute range.
- **LENGTH** sets the playable length from 1 to 64 on SEQUENCE and GENERATE.
  Shortening a pattern retains later steps; they can be restored by lengthening.
- Tap a pitch row to enter that note in the column. Tap the same note again to
  make a rest. A new note retains the column's stored octave.
- Octave rows **+3 / +2 / +1 / -1** are relative to **BASE OCTAVE** in SEQTOOLS.
  Tap the selected octave again for the base octave. MIDI limits remain 0–127.
  Base 3 means MIDI 36, shown as C2 in the NOTE readout (MIDI octave convention).
- **SLIDE / ACCENT / VIBRATO** have separate lanes. An orange indicator follows
  playback. **FOLLOW** changes the visible page as playback crosses page edges.
- Set **SEQUENCER** on, press MPC Play and verify step 16→17, 32→33, 48→49,
  64→1. Stop and start, change tempo, and test a loop in the MPC timeline.

## ARTIC

**EDIT STEP 1-64**, **NOTE** and **TRANSPOSE** provide precise Q-Link editing.
The eight lanes edit **GATE / SLIDE / ACCENT / VIBRATO / TIE / MUTE / HAMMER /
REVERSE** for all 16 columns on the chosen STEP PAGE.

- GATE enables a note; turning it off makes a rest.
- TIE makes a tied rest that extends the preceding note.
- SLIDE glides into the next note. HAMMER is legato with instant pitch change;
  enabling either clears the other.
- ACCENT uses the accented velocity. MUTE is a short/choked note, not a rest;
  enabling either clears the other.
- REVERSE reverses the note envelope; use **ENGINE = JC303 0.14** for the new
  MUTE / HAMMER / REVERSE synthesis behavior. The unchanged legacy voice does
  not acquire those DSP capabilities merely by choosing the new sequencer.
- VIBRATO alternates between the note and an upward pitch offset with smoothing;
  rate and spread are in SEQLIVE. This is an independent implementation, not a
  claim of identical Phoscyon DSP.
- **TIMING = Legacy 16 / New 64**: chunks from versions 1–5 retain the old
  16-step clock/event behavior until explicitly upgraded. New Init uses New 64.
  Old factory programs intentionally preserve their legacy sound/timing.

## SEQTOOLS

- **SHUFFLE** 50–75%: 50 straight; higher values delay alternate steps.
- **STEP LENGTH** 1–100%: ordinary gate duration; slide/tie can extend it.
- **STEP SCALE** 1/64 through 1/1 and **MODIFIER** Full / Dotted / Triplet
  change step duration relative to MPC tempo.
- **PITCH LENGTH** 0 disables pitch polymeter; N cycles the first N note pitches
  across the rhythm. Rests/ties/attributes retain their rhythm positions.
- **REVERSE PLAY** reverses traversal. Per-step REVERSE combines with it.
- **303 CPU TIMING** models upstream 303 event latency/IRQ timing. It is a
  musical timing effect, not a CPU performance optimization.
- **UNDO / REDO** retain 16 step-data edits. Timing/length settings are not
  included in undo. Loading a project clears the transient history.
- **COPY PATTERN / PASTE PATTERN** copy the 64 stored steps within this instance;
  clipboard/history are transient and not saved in the MPC project.
- **SHIFT LEFT / SHIFT RIGHT** rotate all step attributes in the active length.

## SEQLIVE

- **GATE / SLIDE / ACCENT / VIBRATO OVERRIDE**: Pattern uses stored flags; Off
  suppresses that feature; On forces it. Overrides do not rewrite the pattern.
- **VIBRATO RATE / VIBRATO SPREAD** set the alternating pitch movement.
- **PATTERN SHIFT** rotates steps by the change in its value. **PITCH ROTATION**
  rotates only the pitches of note steps; flags and rhythm stay in place.
- **COPY PAGE / PASTE PAGE / CLEAR PAGE** operate on the selected 16-step page,
  including stored steps beyond the current playable length.
- **NOTE -1 / NOTE +1** transpose stored pitches. An operation that would exceed
  MIDI note limits is refused as a whole, following the upstream behavior.

## Generate, record and persistence

Choose **LENGTH 1-64** before **GENERATE**. Generation changes only the active
length. Verify at lengths 7, 17, 33 and 64; later edited steps must survive a
shorter generation. Fill, accent, slide, tie, tones, low note and range retain
their existing meanings. Recording advances and wraps at the active length;
sustain pedal can record a rest. New flags are entered in ARTIC/SEQUENCE.

Save an MPC project/preset with edits on all four pages, a short active length,
shuffle, reverse and pitch length. Reload and verify all settings/notes. Load
an older project and verify **TIMING = Legacy 16** and the original sound.
The new JC303-MPC-7 chunk preserves all 64 steps. Older plugin versions cannot
read this new chunk: keep a copy of older projects before saving new edits.

Key/scale highlighting is available on SEQKEY from 0.4.1: 12 roots and 19 scales plus Off, stored in JC7. It does not alter existing notes or generator pitches. JC6 imports preserve their patterns with guide Off.
No scale snapping, Phoscyon pattern-bank import, arp or Phoscyon effects are
included in this candidate. Other upstream synthesis/modulation/import roadmap
items remain separate work.
