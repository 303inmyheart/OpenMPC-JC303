# JC303 MPC 1.0.0 — Gerätetest ausstehend

Testplan fuer die auf Live III installierte 1.0.0. Bestaetigte Ergebnisse und
Grenzen stehen in TESTING.md; nicht jeder Punkt ist bereits vollstaendig getestet. Herstellerkorrektur
und 18 originale Factory-Sounds zusätzlich zu den zwölf bisherigen Programmen.

1. Vor dem Update ein eigenes Projekt mit einer 0.7.4-Instanz speichern. Nach
   der Installation per OpenMPC Installer muss genau eine JC303-MPC-Installation
   unter 303inmyheart erscheinen; das alte Projekt muss unverändert laden.
2. In der nativen MPC-Presetwahl alle 30 Einträge prüfen. Die ersten zwölf
   behalten Namen und Reihenfolge. Die originalen 18 beginnen mit Open303 Clean
   und reichen von Dice Acid bis Wasp Sting.
3. Alle Originalsounds kurz über Pads anspielen. Squelch verwendet Spicy;
   Super JP den Alpha-Juno-Oszillator; Muff Stomp Vintage Fuzz (Modell 36),
   Pre VCA und Disperse. MODS und Unterbereiche sind entsprechend aktiviert.
4. Eigene Sequenz und Tuning einstellen, dann im VST-Programmenü Originalsounds
   umschalten: Muster/Tuning sollen bleiben. Komplette XPL-Dateien stellen wie
   übliche native MPC-Presets ihren vollständig gespeicherten Zustand wieder her.
5. Einen Factory-Sound ändern, als MPC-Preset/Projekt speichern und erneut
   laden: Änderungen und gewählter Sound dürfen nicht auf Init zurückspringen.
6. Sounds → Instruments: die 30 XPL-Dateien laden. Erneutes Öffnen über das
   Thumbnail bleibt ein getrenntes, bisher zurückgestelltes Host-Thema.
7. Zurück auf Init: MODS und Drive aus, Sequencer aus. Q-Link-Zuordnung und
   Sequencer-Touch bleiben unverändert.

Anschließend Benchmark mit genau dieser finalen ARM64-Datei im leeren Projekt.
Bisherige 0.7.4-Messungen gelten ausschließlich für das alte Binary.

## PRESET INFO und vorbereiteter Altprojekt-Test

- Der Maintainer hat ein Projekt mit der voruebergehend installierten 0.7.4
  angelegt. Jetzt ist wieder 1.0.0 installiert: bitte dieses Projekt oeffnen und
  Klang, Sequencer, Drive und Werte vergleichen. Ein moeglicher Fehler ist
  getrennt vom erfolgreich migrierten eigenen Preset zu untersuchen.
- Auf SYNTH und einer weiteren Seite PRESET INFO oben in der schwarzen Leiste
  antippen: Hinweisfenster muss ueber den Q-Link-Rahmen liegen.
- Aussen neben bzw. unter das Hinweisfeld tippen: Fenster muss schliessen.
  Regler, Modellliste und normale Touch-/Q-Link-Bedienung danach pruefen.

Die PRESET INFO-Anzeige ist auf Wunsch komplett auf Englisch umgestellt und
ausfuehrlicher erklaert. Wiederherstellen aus einem alten Projekt und Speichern
als neues natives MPC-Preset als einfacheren Weg zusaetzlich testen.
