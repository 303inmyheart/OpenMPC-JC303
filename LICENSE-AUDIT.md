# JC303 MPC 0.3.0 license audit

Pinned new upstream: midilab/jc303 v0.14.0,
`b0ff26011f80e45c7796cad7b29fbc7f347b9337` (2026-11-14).

This release is a combined GPLv3/AGPLv3 work, not solely GPL-3.0-only.
The MPC integration and the previous JC303-derived parts retain GPL-3.0-only.
New upstream files explicitly marked `AGPL-3.0-or-later` retain that license:
ARP4075, DiodeLadder, MS20, Roland, SEM and Wasp filter headers/implementations;
CircuitSolve, FilterType, ResonanceMap, TB303OutputNetwork and LFO files.
The full AGPL text is in src/dsp/v014/open303/LICENSE-AGPL-3.0.
The older Open303 MIT notice remains in each engine's LICENSE. File-specific
notices take precedence over any broad description of that directory as MIT.

The combined distribution is labelled `GPL-3.0-only AND AGPL-3.0-or-later`.
Section 13 of GPLv3 / AGPLv3 expressly permits combining those versions. This
does not relicense third-party GPL-only files as AGPL, or AGPL files as GPL-only.
Corresponding source, modifications and build/install scripts must accompany
or be available alongside a matching binary release; retain notices and both
license texts. The AGPL network-interaction provision must also be respected if
a modified covered program provides such interaction. Merely downloading a ZIP
is not remote interaction with a running synth.

References: https://www.gnu.org/licenses/gpl-3.0.html#section13 and
https://www.gnu.org/licenses/agpl-3.0.html#section13 .

Unchanged dependencies: Aciduino (MIT), MPC wrapper (MIT), math_approx
(BSD-3-Clause), RTNeural and xsimd (BSD-3-Clause), nlohmann JSON (MIT), ChowDSP
RNN integration (GPL-3.0), DejaVu fonts (bundled FONT-LICENSE). Existing GuitarML
model files are unchanged. No JUCE, proprietary VST SDK or upstream desktop
artwork is added by this milestone. The AnaMark parser is vendored as upstream
source with the new engine but not used for file import in this milestone.

Source provenance, local namespace/safety adaptations and the old engine pin
are recorded in src/VENDORED.md. Before any public binary release, publish the
matching adapted source and update release/catalog metadata and license notices.
This audit records the source notices and our compliance plan; it is not a
separate grant of rights from any upstream author.
