# JC303 MPC 0.7.0 – Drive und Navigation

Entwicklungsbuild für den nächsten öffentlichen Release. Kein CPU-/Store-Benchmark.
Native/ARM64-Funktionstests sind keine Bestätigung der MPC-Touch-/Hörprüfung.

## Sechs Haupttasten

1. SYNTH: die acht normalen 303-Klangregler.
2. MODS: Circuit → OSC/ENGINE → FILTER → LEGACY.
3. LFO: LFO → MATRIX1 → MATRIX2 → MATRIX3 → MATRIX4.
4. SEQUENCE: Piano → ARTIC → TOOLS → LIVE → KEY → GENERATE.
5. DRIVE: Modell, Gain, Mix, Disperse und Position.
6. SETUP: Synthesekern, globaler MODS-Master und alle Bereichs-/Slot-Schalter.

Ein erneuter Druck derselben Funktionstaste schaltet deren Unterseiten durch.
Die alten Legacy-Filterregler sind auf LEGACY; sie gelten für den alten Kern.
Der Generator gehört jetzt zum Sequencer. Vorhandene Parameter-IDs und gespeicherte
Werte bleiben bestehen. Zur Modmatrix weiterhin JC303 0.14 und MODS MASTER wählen.

## Q-Links

Nur sichtbare Drehregler sind zugeordnet; Drive MODEL bleibt als ausdrücklich
gewünschte Ausnahme zugeordnet. Ein/Aus-Schalter, Taster, sonstige Auswahllisten,
Schrittraster und Statusfelder haben keine Q-Link-Zuweisung. Der zusätzliche
Sequencer-STOP bleibt per Touch sowie als Automationsparameter verfügbar.
Hardware-STOP/All Notes Off bleibt unverändert verfügbar.

Die Bänke folgen den Reglergruppen von links nach rechts und oben nach unten.
Auf SYNTH:

- Bank 1: Wave Morph / Tuning / Cutoff / Resonance.
- Bank 2: Env Mod / Decay / Accent / Volume.

DRIVE: Model / Level-Gain / Dry-Wet / Disperse in einer Bank.
Auf einer Matrix-Seite gibt es nur Amount als Drehregler; Source, Destination
und Slot-Schalter werden dort per Touch gewählt. SETUP ist ohne Q-Link-Bindings.

Der Rahmen muss alle Namen und Wertfelder seiner Bank vollständig einschließen.
Bitte besonders SYNTH beide Bänke, MODS alle drei Reglerreihen, FILTER, LFO und
DRIVE prüfen. Drive-Modellfenster muss weiterhin über dem Rahmen liegen und
sich durch Modellwahl oder Tippen außerhalb schließen lassen.

## Pre-VCA und Disperse

Für Pre-VCA in SETUP die Engine JC303 0.14 wählen. DRIVE einschalten und warten,
bis MODEL STATUS bereit meldet. Unter DRIVE POSITION:

- Post VCA: bisheriger Audioweg, Drive nach der Lautstärkehüllkurve.
- Pre VCA: Drive nach dem Filter, vor Lautstärkehüllkurve und Volume. Die
  Hüllkurve formt den bereits verzerrten Klang. Vergleich besonders mit kurzen
  Amp-Decay-Werten, verschiedenen Modellen und kräftigem Gain.
- ACTIVE ROUTING zeigt den tatsächlich verwendeten Weg. Der Legacy-Kern bleibt
  auf Post VCA; eine gespeicherte Pre-Auswahl wird dort als Legacy-Fallback
  angezeigt. Beim Wechsel ist eine kurze Ab-/Aufblendung vorgesehen.

DISPERSE verschiebt die Phasen von Teiltönen vor dem Drive. Das verändert die
Wellenformspitzen und damit die Verzerrung. 0 = Off, darüber Eckfrequenz bis
3000 Hz (untere Grenze nahe 150 Hz). Es ist kein EQ-Pegelregler. Wie im Original
liegt die Phasenbearbeitung auch vor dem Dry-Anteil des eingeschalteten Drive.
Mit moderatem Gain beginnen, Pre und Post vergleichen, während Noten laufen
langsam drehen und mehrfach zwischen Off und aktiven Werten wechseln.

Drive aus muss wieder trocken spielen. Keine doppelte Drive-Bearbeitung im
Pre-Modus. Hardware-STOP und MIDI-Panic in beiden Positionen prüfen.

## Projekte und Regressionen

JC10 speichert Position und Disperse. Alte Projekte laden mit Post VCA und
Disperse Off. Frühere Plugin-Versionen können JC10 nicht laden.

Projekt mit Pre VCA, Disperse, ausgeschalteten Modulationsslots und eigenem
64-Schritt-Pattern speichern/laden. Alle Werte, Noten und Betriebsarten müssen
bleiben; ein eingerasteter Note Trigger darf nach dem Laden nicht weiterlaufen.
Touch-Raster in SEQUENCE/ARTIC, Scale-Markierungen und Stop/Gate/Trigger mitprüfen.
