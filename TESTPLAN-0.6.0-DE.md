# JC303 MPC 0.6.0 – Bereichsschalter und erweiterte Mods

Entwicklungsbuild für den nächsten öffentlichen Release, kein Store-Benchmark.
Die 0.5.0-Funktionen wurden vom Nutzer als funktionierend bestätigt.

## Schalter pro Bereich

- MODS MASTER bleibt auf der MODS-Hauptseite der gemeinsame Hauptschalter.
- CIRCUIT MODS oben auf derselben Seite: erweiterte Hüllkurven/Slide/Drive/Bass,
  Accent Sweep und globales Reverse Gate. Aus: die normalen 303-Zeiten gelten.
- MODS → ENGINE: OSC MODS schaltet auf den normalen TB-303-Oszillator zurück.
  Die Auswahl „JC303 0.14“ bezeichnet weiterhin den Synthesekern und bleibt stehen.
- MODS → FILTER: FILTER MODS umgeht die zusätzlichen Filtermodelle/-formungen
  und verwendet den normalen TeeBee-Filter. Der Synthesefilter bleibt im Signalweg.
- MATRIX1–4: jeder Slot hat seinen eigenen SLOT-Schalter. LFO / SLOT 1 auf der
  LFO-Seite steuert denselben Schalter wie MATRIX1.

Alle Schalter haben Q-Links und speichern ihren Zustand. Einstellungen bleiben
beim Ausschalten erhalten. Ein ausgeschalteter Bereich sperrt nicht die anderen.
Ein deaktivierter Matrix-Slot 1 bleibt auch bei LFO-Depth-Modulation ausgeschaltet.
Alle Bereiche alter Projekte werden zunächst eingeschaltet übernommen; neue
Effekte bleiben neutral. MODS MASTER aus umgeht auch erweiterte Filterformung.

## Neue LFO-Regler – JC303-0.14-Engine und MODS MASTER an

- START PHASE: 0–360 Grad, verwendet beim Zurücksetzen durch KEY SYNC.
- KEY SYNC: echte Notenstarts starten den LFO bei der gewählten Phase neu.
  Gebundene Slide-Noten setzen ihn gemäß Originalengine nicht neu zurück.
- CONTOUR: −100 bis +100 %, negative Werte für Lag/Slew, positive für die
  Formung des Verlaufs; 0 % neutral.

Test: Pitch als Slot-1-Ziel, Depth ca. 40 %, KEY SYNC an. Gleiche kurze Note
mehrfach spielen; dann START PHASE von 0 auf 180 Grad ändern. Die Modulation
beginnt an einer anderen Stelle. Ohne KEY SYNC läuft der LFO frei. Positive und
negative CONTOUR-Werte vergleichen; Slot-1-Bypass auch auf MATRIX1 gegenprüfen.
Andere Matrix-Slots mit LFO als Quelle benutzen denselben Modulator.

## Neue Circuit-Regler – JC303-0.14-Engine, MASTER und CIRCUIT an

- ACCENT SWEEP: 15–2000 ms; verändert die Lade-/Entladezeit der Accent-Schaltung.
  Langsame Werte lassen Folgen akzentuierter Noten aufbauen. 15 ms ist neutral.
- REVERSE GATE: 0–100 %, Überblendung zum rückwärts anschwellenden Amp-/Filter-
  Verlauf. Betrifft global die Noten. Der vorhandene REVERSE-Schritt bleibt als
  gesonderte Sequencer-Artikulation bestehen.

Test: mehrere akzentuierte Schritte hintereinander, hörbarer ENV MOD und Resonanz.
Accent Sweep erhöhen und mit 15 ms vergleichen. Reverse Gate langsam aufdrehen.
CIRCUIT aus: beide Zusatzregler dürfen nicht wirken, behalten aber ihre Werte.

## Hardware-Stop und gespeicherte Einstellungen

Note Trigger starten, Note loslassen, Hardware-STOP drücken: Wiedergabe endet,
Status geht zu „Waiting for Note“. Die Modusauswahl bleibt Note Trigger; eine
neue Note darf erneut starten. Auch bei bereits gestopptem MPC-Transport testen.
Die vorhandene All-Notes-Off-Behandlung ist dafür über den echten VST-MIDI-Pfad
geprüft (CC123); CC120 stoppt zusätzlich sofort. Hardwareverhalten selbst bitte
bestätigen. Der Touch-/Q-Link-STOP auf TOOLS bleibt vorhanden.

Projekt mit ausgeschalteten Bereichen und veränderten neuen Reglern speichern,
laden und alle Werte prüfen. Keine gehaltene oder eingerastete Note darf nach
Laden weiterlaufen. Bestehende Touch-Raster, Q-Link-Rahmen und Drive-Menü mitprüfen.
JC9 speichert die Erweiterungen; frühere Plugin-Versionen können JC9 nicht laden.
