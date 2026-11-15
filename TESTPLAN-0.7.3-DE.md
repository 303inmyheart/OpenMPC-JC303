# JC303 MPC 0.7.3 – Hör- und Bedienprüfung

Die Funktionen und Oberfläche entsprechen 0.7.0. Diese Version reduziert die
CPU-Spitzen bei Parameteränderungen; es gibt keine neuen Regler oder Seiten.

1. Einen bestehenden Klang laden und hören, besonders Square Drive, MODS und
   Circuit/Osc/Filter einzeln umschalten. Der Klang soll gleich bleiben.
2. Auf FILTER alle acht Modelle wählen, Cutoff/Resonance und Bias bewegen;
   auch Hi-Fi/FM und schnelle Modellwechsel prüfen.
3. SEQUENCE und ARTIC: Noten und Flags im Stop und während Play ändern,
   Seiten und Tonleiter wechseln. Anzeigen sollen ohne Zurückspringen folgen.
4. DRIVE: Blackstar HT40 Gainknob SM57Mic, Pre/Post und Disperse prüfen;
   Modellauswahl öffnen und per Außentipp schließen. Q-Link-Rahmen kontrollieren.
5. Projekt speichern und neu laden; Parameter und Pattern sollen erhalten bleiben.

Alle bestehenden 0.7.0-Funktionstests gelten weiter. Hardware-Benchmarks:
Standard WARN (p99 31,5 %, max. 39,0 %), zusätzlicher Vollparameter-Stress WARN
(p99 31,5 %, max. 44,5 %). Das sind Audio-Block-Zeiten auf einem Kern, nicht
Werte der MPC-Projektanzeige. Rohdaten: benchmarks/0.7.3/. Noch kein öffentlicher
Release; diese Bedien- und Hörprüfung kann nicht durch Offline-Tests ersetzt werden.
