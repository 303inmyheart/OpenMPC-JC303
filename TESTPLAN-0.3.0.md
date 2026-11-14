# JC303 MPC 0.3.0: Schritt 1 testen

Dieser Build bringt die Klangerzeugung von JC303 v0.14.0. Die folgenden Namen
entsprechen den Reitern und Beschriftungen im Plugin. Es ist noch keine
vollständige Umsetzung aller Funktionen des neuen Desktop-Releases.

## Vorbereitung

Eine neue JC303-MPC-Instanz öffnen. Auf **ENGINE** bei **ENGINE** den Eintrag
**JC303 0.14** wählen. Auf **MODS** den Schalter **MODS** einschalten. Die
neuen Modelle sind wie im Original an MODS gebunden; bei ausgeschalteten MODS
arbeitet die 0.14-Engine mit dem einfachen TeeBee-/Open303-Grundklang.
Zunächst DRIVE ausgeschaltet lassen, VOLUME auf etwa **-18 dB** stellen
(die Filter können bei viel Resonanz hohe Pegel liefern) und auf SYNTH Cutoff ungefähr in die Mitte,
Resonance etwa auf 30–50 % stellen. Pads spielen oder den bisherigen Sequencer
starten. Die übrigen bekannten SYNTH-/MODS-Regler bleiben verfügbar.

## ENGINE: Oszillatoren

| Beschriftung | Auswahl / Test |
|---|---|
| ENGINE | **Legacy 0.2.3** und **JC303 0.14**. Wechsel stoppt laufende Noten. |
| OSCILLATOR | **TB-303**, **Alpha Juno**, **Phaseshaper** müssen unterschiedliche Klänge liefern. |
| 303 SQUARE | Bei TB-303, Wave Morph Richtung Square: **Open303** gegen **Circuit** vergleichen. Circuit hat die neue Tonhöhen-/Pulsform-Modellierung. |
| JUNO WAVE | Bei Alpha Juno: **1 Saw**, **2 Oct**, **3 PW**, **4 Hi Oct**, **5 Dual** durchschalten. |
| SUPER DETUNE | Bei Alpha Juno die Verstimmung von einer Stimme bis zum breiteren Supersaw-Klang erhöhen. |
| SUPER MIX | Balance zwischen mittlerer und seitlichen Supersaw-Stimmen; mit erhöhtem Super Detune testen. |
| PHASESHAPER | Bei Phaseshaper **Tri Mod**, **Ripple Saw**, **Formant** vergleichen; Wave Morph verändert die Form. |
| SHAPE DRIFT | Bei Phaseshaper, besonders Ripple Saw/Formant: Bewegung/Schwebung erhöhen. |
| FORMANT SPLIT | Im Formant-Modus die Aufspaltung der Formanten verändern. |
| SUB LEVEL | Den zusätzlichen Bass unter dem Hauptoszillator von 0 aufdrehen. |
| SUB OCTAVE | Mischung zwischen zwei Oktaven tiefer (0 %) und einer Oktave tiefer (100 %); Sub Level muss über null liegen. |

**SQUARE DRIVE** bleibt auf MODS: im Circuit-Square-Modus beeinflusst es die
Pulsform/Sag, im Open303-Square-Modus den bisherigen Waveshaper.

## FILTER: Modelle

**FILTER MODEL** nacheinander durchschalten. Alle Einträge müssen Klang liefern;
bei gleichem Cutoff/Resonance sind Pegel und Resonanzcharakter unterschiedlich.

| Modell | Vorbild / Bedeutung |
|---|---|
| TeeBee | Bisheriger Open303-303-Filter |
| Spicy | Neu überarbeiteter, schaltungsnaher 303-Diodenfilter |
| MissBehave | Korg MS-20 / MS-10 |
| JuneBug | Roland Juno-106 |
| Wasabi | EDP Wasp |
| Hornet | Doepfer A-124 |
| SemiSweet | Oberheim SEM |
| Oddity | ARP 4075 |

## FILTER: Klangformung

| Beschriftung | Erwartete Wirkung / Voraussetzung |
|---|---|
| LP / BP / HP | Morph zwischen Tiefpass (0 %), Bandpass (50 %) und Hochpass (100 %). Bei Spicy und den neuen Schaltungsfiltern testen; TeeBee bleibt Tiefpass. |
| SATURATION | 0 bis +16 dB Ansteuerung des Filters, mit Pegelkompensation. Klang wird stärker gesättigt; es ist kein normaler Lautstärkeregler. TeeBee reagiert hier nicht hörbar. |
| RES TRIM | Bei Spicy die Resonanz-Kalibrierung zwischen 0,85× und 1,15× verändern; 1,00× ist nominal. |
| FILTER FM | Audiofrequente Cutoff-Modulation, besonders bei höherer Resonanz hörbar. |
| TENSION | Bei Spicy mit **Hi-Fi**: -100 % bis +100 %, Mitte 0 %. Ändert das Verhalten der Resonanz bei stärkerer Anregung. |
| BIAS | Asymmetrie/Sättigungscharakter. Mit SATURATION über 0 dB bei Spicy, MissBehave, Wasabi/Hornet und Oddity testen. JuneBug/SemiSweet halten diese Asymmetrie intern neutral. |
| RAIL | Modellabhängige Begrenzung/Versorgungsspannung; 100 % entspricht der nominalen Einstellung. Bei Spicy vor allem in Hi-Fi und stärkerer Sättigung testen. |
| SPICY QUALITY | **Standard** und **Hi-Fi** für Spicy. Hi-Fi modelliert mehr Nichtlinearitäten und kann mehr CPU brauchen. |

**BASS COMP** bleibt auf MODS und wirkt im neuen Spicy-Modell. **LEGACY FILTER**
und **LEGACY DRIVE** auf MODS gehören ausschließlich zur alten Engine. Für die
neue Engine FILTER MODEL und SATURATION auf FILTER verwenden.

## Speichern und Altprojekte

1. Ein altes Projekt / altes gespeichertes JC303-Preset laden: ENGINE muss
   **Legacy 0.2.3** anzeigen. Der alte Filter und der Klang sollen erhalten bleiben.
   Auch die früheren Varianten **Diode Oct.** und **Diode** gezielt prüfen.
2. Einen neuen Klang mit JC303 0.14, anderem Modell, Sub-Oszillator und
   Filterbearbeitung speichern. Nach erneutem Laden müssen Engine, Einstellungen
   und Klang wiederkommen.
3. Die zwölf bisherigen MPC-Presets sind unverändert als Legacy-Klänge verfügbar.
   Nach Auswahl eines solchen Presets für neue Funktionen wieder auf ENGINE
   **JC303 0.14** wählen. Der neue JSON-Werksklang-Browser folgt später.
4. Sequencer mit Noten, Accent und Slide laufen lassen. Er muss beide Engines
   ansteuern; die neuen v0.14-Sequencerfunktionen sind hier noch nicht enthalten.
5. Danach DRIVE zuschalten und ein bekanntes Amp-Modell vergleichen. Die
   bisherigen 37 Modelle und die optimierte Berechnung bleiben erhalten.

Bitte besonders auf Knacken, Hänger, stumme Modelle, unerwartete Parametersprünge,
CPU-Spitzen und korrekte Beschriftungen achten. Beim Berichten Reiter, Modell,
Regler und ungefähr die Stellung nennen. Offline-Tests ersetzen diesen MPC-Test
nicht.

## Noch nicht Teil dieses Schritts

Vier-Slot-Modulationsmatrix und zusätzliche LFO-Bedienung; neue Sequencerfunktionen
wie Reverse, Mute/Hammer, Swing, Polymeter und Undo/Redo; Pre-VCA-Drive/Disperse;
neue JSON-Werksbänke und Dateiimport; AnaMark-.tun-Import. Das kommt in späteren
Schritten. Neue Reiter und Regler verwenden weiterhin das bestehende Acid-Design.
