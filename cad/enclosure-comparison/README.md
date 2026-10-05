# Five enclosure designs — INGE / AFTER HOURS

The five digital prototype kits are complete: editable OpenSCAD, separate STL
parts, acrylic outlines, assembled/exploded previews, an offline interactive
viewer, quantities, and assembly instructions. Revision 3, 5 October 2026.
**Physical hardware fit is still unverified.** Fit-test the small parts with the
actual panel, ESP32 and cables before printing a complete case.

Open [viewer.html](viewer.html) in a desktop browser. It works without internet;
use the five tabs, Front/Rear, Open back and Exploded controls. The viewer embeds
actual STL meshes; the panel and PCB are nominal component envelopes. Preview
LED artwork is illustrative. [comparison.png](comparison.png) shows all five.

| Design | Case width × height × depth, mm | Acrylic, mm | Cable tray height | Estimated stored cable* |
| --- | --- | --- | --- | --- |
| [Line](kits/01-line.zip) | 190 × 110 × 94.5 | Cut 164 × 84 × 1 | 14 mm | 0.2 m |
| [Orbit](kits/02-orbit.zip) | 202 × 122 × 96.5 | Cut 164 × 84 × 1 | 20 mm | 0.4 m |
| [Gallery](kits/03-gallery.zip) | 206 × 156 × 96.5 | Full 180 × 130 × 1 | 20 mm | 0.4 m |
| [Vault](kits/04-vault.zip) | 206 × 156 × 122.5 | Full 180 × 130 × 1 | 40 mm | 1.0 m |
| [Console](kits/05-console.zip) | 202 × 122 × 108.5 | Cut 164 × 84 × 1 | 30 mm | 0.8 m |

Depth includes the rear port plate, excludes screw heads and cradle lips.
Feet add 4 mm below the case; their lips extend beyond the front/rear faces.
*One bundle with 6 mm effective diameter; estimate excludes service tails and
assumes a 0.2 m oval per turn. It is a planning allowance, not a measured cable fit.
Vault has the largest wiring compartment; Line is the smallest enclosure.

The confirmed component list is **one panel, one ESP32, acrylic, and wiring**.
The power adapter stays outside. No speaker or extra electronics are assumed.
The default rear plate passes the existing power and USB leads through two
separate openings, so a USB extension is not required. The optional
`ports_usb_c.stl` is only for a separately selected round panel-mount USB adapter.

- [Print and assembly guide](PRINT-GUIDE.md): quantities, fasteners and fit steps.
- [French workshop sheet](FABLAB-FR.md): handoff for the fablab.
- [Audit](AUDIT.md): corrections, evidence and unresolved physical measurements.
- [Validation report](validation.json): mesh, assembly and clearance results.
- [All five kits and viewer](five-enclosure-kits.zip): complete portable handoff.

The older `cad/inge-after-hours` folder is the first single-case prototype.
Use this comparison family for these five designs; its acrylic is 1 mm, whereas
the older model assumed 2 mm.

## Reproduce or adjust

Use Python 3.11+ and OpenSCAD. In an isolated Python environment:

```sh
python3 -m venv /tmp/matrix-cad-venv
/tmp/matrix-cad-venv/bin/pip install -r cad/enclosure-comparison/requirements.txt
/tmp/matrix-cad-venv/bin/python cad/enclosure-comparison/build.py
/tmp/matrix-cad-venv/bin/python cad/enclosure-comparison/validate.py
/tmp/matrix-cad-venv/bin/python cad/enclosure-comparison/render.py
/tmp/matrix-cad-venv/bin/python cad/enclosure-comparison/make_viewer.py
/tmp/matrix-cad-venv/bin/python cad/enclosure-comparison/package_kits.py
```

Set `OPENSCAD='/path/to/openscad'` if it is not on PATH. A portable official Node
WASM build also works: `OPENSCAD='node /path/to/openscad.js'`. Exports in this
revision used OpenSCAD 2026.10.03. Pinned Python dependencies record the validation
environment; use compatible versions if a wheel is unavailable for your Python.

`build.py --help` lists measured component overrides, for example
`--pcb-length 58 --pcb-width 29 --pcb-hole-length 52 --pcb-hole-width 23`.
Those numbers are current provisional defaults, not measurements of your board.
OpenSCAD `family.scad` also exposes these parameters. After changing them, rerun
all five steps: the manifest, meshes, visualizations and checks must agree.
The mounting slots accept 30–64 mm hole spacing along the board and 0–32 mm
across it; check actual board edge clearance and screw size separately.

Export individual parts, never an assembled or exploded mesh as one print.
All parts fit the assumed 220 × 220 × 250 mm build volume with 5 mm brim allowance
in the supplied orientation. Machine-specific slicing and print settings still
belong to the workshop. No machine G-code is distributed.
