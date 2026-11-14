# JC303 MPC 0.4.0 — Test auf der MPC

Entwicklungskandidat für den neuen Sequencer. Der vollständige englische
Kontrollenkatalog steht in [TESTPLAN-0.4.0.md](TESTPLAN-0.4.0.md).
Ein Store-Benchmark folgt erst beim endgültigen Release-Build.

1. **Neue Instanz / Init:** SEQUENCE mehrfach drücken. Es wechselt zwischen
   **SEQUENCE → ARTIC → SEQTOOLS → SEQLIVE → SEQKEY**. MODS / ENGINE / FILTER und der
   Preset-Browser sollen weiterhin funktionieren.
2. **Noten eingeben:** Im Raster eine Notenreihe antippen. Grün bedeutet Note;
   dieselbe Note erneut antippen ergibt eine Pause. Die vier Zeilen darüber
   wählen **+3 / +2 / +1 / -1 Oktaven** relativ zu BASE OCTAVE. Erneut auf die
   gewählte Oktave tippen setzt die Grundoktave. Der Rahmen markiert den
   bearbeiteten Schritt, Orange oben und der durchgehende orange Strich zeigen den laufenden Schritt
   (ab 0.4.1).
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
    verwenden JC303-MPC-7; ältere Plugin-Versionen können dieses Format nicht
    laden. Alte Projekte deshalb vor dem erneuten Speichern als Kopie behalten.

**Neu in 0.4.1: SEQKEY**

Unter ROOT NOTE / KEY den Grundton und unter SCALE / MODE die Skala wählen.
Alle zwölf Grundtöne sind vorhanden, einschließlich der #/b-Bezeichnungen.
B entspricht dem deutschen H. Major bedeutet Dur/Ionisch, Minor natürliches
Moll/Äolisch. Melodic Minor verwendet die aufwärts/Jazz-Moll-Form.

Enthalten: Major, Minor, Harmonic Minor, Melodic Minor, Dorian, Phrygian, Lydian,
Mixolydian, Locrian, Major Pentatonic, Minor Pentatonic, Minor Blues, Major Blues,
Whole Tone, Dim Half-Whole, Dim Whole-Half, Phrygian Dominant, Double Harmonic
und Chromatic. Off schaltet die Markierung aus.

Zum Test A + Minor wählen: A, B, C, D, E, F und G müssen im Raster hervorgehoben
sein; die Kopfzeile zeigt A Minor. Danach D + Minor wählen: A# (Bb) gehört nun
dazu, B nicht. Alle vier STEP PAGE-Seiten kontrollieren. Vorhandene Noten sollen
bei jedem Wechsel erhalten bleiben und gleich klingen; auch Noten außerhalb
der Skala bleiben spielbar. Generate wird durch diese Auswahl nicht eingeschränkt.
Das Projekt speichern/neuladen: Key und Scale müssen erhalten bleiben.
Projekte aus 0.4.0/JC6 behalten ihre 64 Schritte; die Skalenanzeige beginnt Off.

Die Touch-Eingabe für Noten, Oktaven sowie Slide-/Accent-/Vibrato-Schalter bitte
noch einmal auf der MPC testen; 0.4.1 korrigiert deren native Button-Bindings.
Beim Abspielen soll ein orangefarbener Strich durch die aktuelle Spalte laufen.

Phoscyon-Bänke, dessen Arpeggiator und Effekte sind nicht Bestandteil des Ports.


**Gezielter Stop/Play-Test für 0.4.1:** In einer neuen Init-Instanz zuerst
LENGTH 16, STEP PAGE 1–16 und FOLLOW Off einstellen. Im Stillstand eine Note
setzen, Play drücken, eine weitere Note hinzufügen und wieder Stop/Play
betätigen. Beide Noten müssen im Raster erhalten bleiben. Bei BASE OCTAVE 3
und einer C-Note müssen die Oktavzeilen +3 / +2 / +1 / -1 in ARTIC → NOTE
72/C5, 60/C4, 48/C3 bzw. 24/C1 ergeben. Ein zweiter bewusster Tap auf dieselbe
Oktavzeile setzt die Grundoktave zurück. Die Touch-Lift-Doppelmeldung einer
einzelnen Berührung darf die Note nicht sofort wieder löschen.


**Neu in 0.4.2: ARTIC und die Schalterzeilen**

Bitte SLIDE, ACCENT und VIBRATO auf SEQUENCE jeweils ein- und ausschalten.
Auf ARTIC anschließend jede der acht Zeilen gezielt antippen. Die betroffene
Zelle soll in derselben Zeile leuchten und sich wieder ausschalten lassen.
TIE ist die Ausnahme in der musikalischen Bedeutung: Es erzeugt eine gebundene
Pause, daher ist GATE dann aus. TIE selbst muss sichtbar an sein. Mit GATE
lässt sich die gespeicherte Note wieder einschalten; dabei geht TIE aus.
