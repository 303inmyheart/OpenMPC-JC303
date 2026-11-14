# JC303 MPC 0.4.0 — Test auf der MPC

Entwicklungskandidat für den neuen Sequencer. Der vollständige englische
Kontrollenkatalog steht in [TESTPLAN-0.4.0.md](TESTPLAN-0.4.0.md).
Ein Store-Benchmark folgt erst beim endgültigen Release-Build.

1. **Neue Instanz / Init:** SEQUENCE mehrfach drücken. Es wechselt zwischen
   **SEQUENCE → ARTIC → SEQTOOLS → SEQLIVE**. MODS / ENGINE / FILTER und der
   Preset-Browser sollen weiterhin funktionieren.
2. **Noten eingeben:** Im Raster eine Notenreihe antippen. Grün bedeutet Note;
   dieselbe Note erneut antippen ergibt eine Pause. Die vier Zeilen darüber
   wählen **+3 / +2 / +1 / -1 Oktaven** relativ zu BASE OCTAVE. Erneut auf die
   gewählte Oktave tippen setzt die Grundoktave. Der Rahmen markiert den
   bearbeiteten Schritt, Orange oben den laufenden Schritt.
3. **64 Schritte:** LENGTH auf 64 stellen. Mit **STEP PAGE** alle vier Seiten
   bearbeiten: **1–16 / 17–32 / 33–48 / 49–64**. Auf jeder Seite eine gut
   erkennbare Note setzen. SEQUENCER einschalten und MPC Play drücken. Auf
   Übergänge 16→17, 32→33, 48→49 und 64→1 achten. **FOLLOW** soll dabei die
   sichtbare Seite wechseln. Stop, Neustart und Tempoänderungen testen.
4. **ARTIC:** **EDIT STEP 1-64**, **NOTE**, **TRANSPOSE** sind die feinen
   Eingaben über Q-Links. Die acht Zeilen bearbeiten die sichtbaren 16 Schritte:
   **GATE, SLIDE, ACCENT, VIBRATO, TIE, MUTE, HAMMER, REVERSE**.
   GATE aus ergibt Pause. TIE ist eine gebundene Pause, die die vorige Note
   verlängert. SLIDE gleitet in die nächste Note; HAMMER bindet mit sofortigem
   Tonhöhensprung. Beide schließen sich gegenseitig aus. MUTE ist eine kurze,
   abgedämpfte Note und schließt ACCENT aus. Für die neuen Klangmerkmale
   **ENGINE = JC303 0.14** verwenden. Die alte Synthese-Engine bleibt unverändert.
5. **Timing:** In SEQTOOLS **SHUFFLE** von 50 auf 60–70 stellen; 50 ist gerade.
   **STEP LENGTH** verändert die normale Gate-Dauer von 1–100%; Slide und Tie
   dürfen sie verlängern. **STEP SCALE** und **MODIFIER = Full / Dotted /
   Triplet** ändern die Schrittdauer. **REVERSE PLAY** spielt rückwärts.
   **303 CPU TIMING** simuliert das Timing der 303-CPU; es spart keine Rechenlast.
6. **Pitch Length:** Ein Pattern mit mehreren verschiedenen Noten eingeben.
   **PITCH LENGTH = 3** lässt drei Tonhöhen gegen das längere Rhythmuspattern
   laufen; **0** schaltet das aus. **BASE OCTAVE** bestimmt die Bezugsoktave
   der Rastereingabe. Standard 3 entspricht MIDI 36, im NOTE-Display C2.
7. **Bearbeiten:** **UNDO / REDO** testen. Die letzten 16 Änderungen an den
   Schrittdaten sind enthalten; Timing-/Längeneinstellungen sind ausgenommen.
   **COPY PATTERN / PASTE PATTERN** kopieren die 64 gespeicherten Schritte.
   **SHIFT LEFT / SHIFT RIGHT** rotieren das Pattern innerhalb seiner Länge.
8. **SEQLIVE:** Die vier **OVERRIDE**-Auswahlen haben **Pattern / Off / On**.
   Sie unterdrücken oder erzwingen Gate, Slide, Accent und Vibrato, ohne die
   gespeicherten Noten zu überschreiben. **VIBRATO RATE / VIBRATO SPREAD**
   verändern eine geglättete, abwechselnde Tonhöhenbewegung. Das ist eine eigene
   Umsetzung, keine identische Nachbildung des D16-Klangs.
   **PATTERN SHIFT** verschiebt ganze Schritte; **PITCH ROTATION** nur deren
   Tonhöhen. **COPY PAGE / PASTE PAGE / CLEAR PAGE** betreffen die sichtbare
   16er-Seite. **NOTE -1 / NOTE +1** transponieren gespeicherte Noten. Würde eine
   MIDI-Grenze überschritten, wird die gesamte Transposition abgelehnt.
9. **Generate:** Auf GENERATE **LENGTH 1-64** vor GENERATE einstellen, z.B.
   7, 17, 33 und 64. Nur diese Länge wird neu erzeugt. Zuvor bearbeitete spätere
   Schritte müssen nach erneutem Verlängern noch vorhanden sein. Fill,
   Accent-/Slide-/Tie-Wahrscheinlichkeit und Tonhöhenbereich wie bisher testen.
   MIDI-/Pad-Recording muss an der aktiven Länge umlaufen.
10. **Speichern:** Ein MPC-Projekt/Preset mit Noten auf allen vier Seiten,
    verkürzter Länge, Shuffle und neuen Flags speichern und neu laden. Alle
    gespeicherten Schritte müssen erhalten bleiben. Ein altes Projekt soll
    **TIMING = Legacy 16** und seinen bisherigen Klang behalten. Neue Projekte
    verwenden JC303-MPC-6; ältere Plugin-Versionen können dieses Format nicht
    laden. Alte Projekte deshalb vor dem erneuten Speichern als Kopie behalten.

Tonart-/Skalenmarkierungen wie A Minor sind weiterhin für später vorgemerkt.
Phoscyon-Bänke, dessen Arpeggiator und Effekte sind nicht Bestandteil des Ports.
