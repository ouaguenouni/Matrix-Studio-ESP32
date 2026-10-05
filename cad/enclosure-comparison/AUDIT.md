# Enclosure audit — revision 3

The previous work contained all five STL sets and static previews but stopped
before producing the referenced print guide, audit, generated viewer or packaged
kits. Its geometry report passed, but some preview and source assembly states
differed and several practical fit questions were not documented.

## Corrections completed

| Finding | Result |
| --- | --- |
| `PRINT-GUIDE.md`, `AUDIT.md` and `viewer.html` were referenced but missing | Complete guides, this audit and the generated standalone viewer are included |
| No portable five-design handoff or per-design download | Each variant has a ZIP with shared CAD source, quantities, previews and guides; the full archive also contains the offline viewer |
| OpenSCAD assembly omitted cradle feet and PCB | Feet and nominal PCB now appear; Python uses the same placement |
| Exploded CAD, image and viewer used different offsets | Viewer offsets come from the checked assembly; CAD and Python exploded placement agree |
| Acrylic coupon grooves had a 0.3 mm roof | Grooves now open from above, with a 1.2 mm floor |
| Keeper used a nominal 2 mm bore as an M2 pilot | Default is now a 1.6 mm pilot, with 1.6/1.8 mm coupon holes; still test the printed fit |
| Board and spacer dimensions were hardcoded in previews | Editable source/build parameters populate the manifest, assembly, checks and viewer |
| Missing STL files could escape the mesh loop | Validation requires the exact expected part set and matching body dimensions |
| Stale source/exports could be packaged as passing | Export records source hash; validation records file hashes; packaging refuses modified checked inputs |
| Only front/rear/keeper head envelopes were checked | Panel, bridge and PCB screw-head envelopes and four PCB bolt passages are checked too |
| Viewer claimed diagnostic slicing without a report | Claim removed; provided diagnostic profile is labelled unverified |

## Digital validation performed

All five kits were regenerated using official OpenSCAD 2026.10.03 Node WASM.
For 65 distinct STL files, validation checks one connected solid, watertightness,
consistent winding, positive volume, Z=0 base and assumed bed/brim clearance.
Assembly checks intersect actual transformed meshes pairwise, including nominal
panel/PCB/acrylic and the documented screw-head envelopes. They also check a
panel connector reserve, USB plug reserve, PCB wiring reserve and four clear
PCB bolt paths. `validation.json` contains the result and its checked file hashes.

The viewer has five design tabs, front/rear views, open-back and exploded modes,
mouse/keyboard rotation and zoom. It embeds actual STL geometry and uses the
same exploded transforms as the validation/render assembly. These visual modes
are review views; they are not STL files intended to be printed as one object.

## Physical measurements and tests still needed

| Item | Evidence/default | Required real check |
| --- | --- | --- |
| Panel | P2.5/80×160 marking in project photos; saved official drawing evidence | Actual outline, connected depth and insert engagement |
| Panel mounting | Outer four drawing M3 centres, 125 × 65 mm | Confirm these inserts on the physical panel revision |
| ESP32 | Photographed USB-C board with corner holes; 58 × 29 / 52 × 23 mm defaults | Board dimensions, hole diameter/spacing, components and underside jumpers |
| Acrylic | Current comparison CAD records owner-confirmed 1 mm thickness | Test actual sheet/pad in the printed recess; older prototype used 2 mm |
| Nut and screw fit | 6.1 × 2.5 mm carrier channels, documented nominal heads | Actual nuts, washers, head heights and printed pilot engagement |
| Wiring | Fixed 20 mm panel connector reserve and 64 × 36 × 38 mm PCB reserve | Plugged cables, slack, connector positions and actual bend requirements |
| Cable cassette | R25 mm oval guide; 6 mm effective bundle estimate | Bundle diameter, usable slack and strap retention without crushing wires |
| Feet | Two printed cradles, pads/tape proposed | Cable-loaded stability and attachment |
| Printing | 220 × 220 × 250 mm assumed bed; 5 mm brim allowance | Workshop machine, slicing/supports, tolerance and finished part quality |
| Electronics in closed case | Existing project power procedure retained | USB upload, Wi-Fi, BOOT/EN service access and sustained temperature |

No thermal certification, electrical redesign, actual print/slice result or
physical fit certification is implied by a passing mesh report. Additional
speaker/power-distribution components are outside the confirmed component list.

## Source evidence

- [Waveshare panel documentation](https://docs.waveshare.com/RGB-Matrix-Px-64x32).
- [Waveshare P2.5 drawing](https://files.waveshare.com/wiki/RGB-Matrix-P2.5-64x32/RGB-Matrix-P2.5-64x32-2D.dwg);
  the existing extraction is preserved in `evidence/matrix-mounting-evidence.json`
  and its diagram. These recorded coordinates are not a new hardware measurement.
- [OpenSCAD official downloads](https://openscad.org/downloads.html), including
  the Node WASM build used to regenerate the models.
- Project `pictures/` and `docs/gift-enclosure-brief.md` provide the original
  hardware context. The original brief and single-case README remain history;
  this folder describes the five-design revision.
