# JC303 MPC 0.5.0 – Modmatrix und Note-Startmodi

Lokaler Entwicklungskandidat für den nächsten Release. Nicht als öffentlicher
Release oder als auf der MPC bestätigt behandeln. Ausgangspunkt: bestätigte 0.4.2.
Kein Store-/CPU-Benchmark in dieser Entwicklungsphase.

## Vorbereitung

Ein bestehendes Projekt zunächst unter anderem Namen sichern. Alte Projekte
behalten ihre bisherigen Start-/Klang-Einstellungen; neue Modulationswege sind
bei ihrer Übernahme neutral. Für die Modmatrix unter MODS → ENGINE die Engine
„JC303 0.14“ wählen und MODS einschalten. Der Sequencer-Startmodus erfordert die
neue 64-Step-Zeitbasis, aber keinen bestimmten Synthesekern und kein MODS.

## Modmatrix

MODS durchschalten: MODS → ENGINE → FILTER → MATRIX1 → MATRIX2 → MATRIX3 → MATRIX4.
Die Q-Link-Bank folgt der jeweils sichtbaren Unterseite.

- MATRIX1: Quelle fest LFO, Amount entspricht dem bisherigen LFO DEPTH.
  Destination und Depth sind mit der LFO-Seite synchron.
- MATRIX2: Quelle fest Env; Amount −100 bis +100 Prozent.
- MATRIX3/4: Quellen Off, LFO, Env, Accent, Key, Random, Cutoff, Env+.
  Amount −100 bis +100 Prozent. Env ist bipolar, Env+ unipolar.
- Ziele: Cutoff, Volume, Pitch, Pulse Width, Resonance, Overdrive, Filter FM,
  Env Mod, Decay, Accent, Waveform, LFO Rate, LFO Depth, Filter Morph.
- Overdrive moduliert das Dry/Wet-Verhältnis eines eingeschalteten DRIVE;
  es verändert den gespeicherten Basiswert nicht. Filter Morph / Filter FM und
  Pulse Width hängen vom gewählten Filter-/Oszillatormodell ab.

Praktischer Start: Slot 3 Source Cutoff, Destination Pitch, Amount zunächst 0,
dann +20 %, anschließend −20 %. Cutoff bewegen und hören. Danach Source Off:
der zusätzliche Einfluss muss verschwinden. Slot 2 Env → Cutoff mit positivem
und negativem Amount vergleichen. MODS aus: Matrix aus. Legacy-Engine: nur der
bisherige LFO-Pfad verfügbar; der Status zeigt dies an.

## Startmodi

Unter SEQUENCE → TOOLS findet sich START MODE mit START STATUS. STOP ist
zusätzlich dem dritten Eintrag der vierten Q-Link-Spalte zugeordnet und als
automatisierbarer Impulsparameter „Stop Note Sequencer“ bereitgestellt. Die
Automationsauswahl und Aufnahme auf MPC OS müssen am Gerät geprüft werden.
Pattern mit mehreren klingenden Schritten vorbereiten, SEQUENCER einschalten.

1. **Host Transport**: entspricht dem bisherigen Verhalten; Play startet, Stop stoppt.
2. **Note Gate/Hold**: Bei gestopptem MPC eine Note/ein Pad halten: Pattern startet
   bei Schritt 1 im MPC-Tempo. Letzte Note loslassen: Sequencer stoppt. Zwei
   Noten halten, nur eine loslassen: läuft weiter. Erneute Note nach vollständigem
   Loslassen: startet erneut bei Schritt 1.
3. **Note Trigger**: Note/Pad antippen: Pattern startet bei Schritt 1 und läuft
   nach dem Loslassen weiter. Weitere Noten während des Laufens starten es nicht
   neu. STOP auf der TOOLS-Seite drücken, MPC-Transport von Play auf Stop beenden
   oder MIDI All Notes Off senden:
   stoppt. Bei bereits gestopptem Host kann eine Note erneut starten.

Triggernoten transponieren das Pattern nicht. Beide Note-Modi wählen nötigenfalls
automatisch die neue 64-Step-Zeitbasis. Bei ausgeschaltetem Sequencer bleiben
Pads normale spielbare Noten. Wechsel des Startmodus beendet laufende Noten.
Die Auswahl wird gespeichert; gehaltene Noten oder ein eingerasteter Trigger
werden beim Laden nicht fortgesetzt. Startstatus muss „Waiting for Note“ zeigen.

## Regressionen

- Touch-Eingaben in SEQUENCE und ARTIC während Stop und Play behalten.
- Slide/Accent/Vibrato und alle ARTIC-Zellen lassen sich ein- und ausschalten.
- Vier Seiten, Schritte 1–64, Key/Scale und Oktaven bleiben korrekt.
- Projekt mit allen vier Slots und Startmodus speichern/laden; Werte bleiben.
- Altes 0.4.2-Projekt laden: alte Klang- und Transportfunktion bleibt bestehen.
- DRIVE-Modellmenü öffnet/schließt weiter und liegt über dem Q-Link-Rahmen.
