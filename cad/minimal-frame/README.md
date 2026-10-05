# Minimal frame — enclosed rear, two printed parts

A close-fitting frame for the 160 × 80 mm display, with a ventilated rear shell.
The ESP32's USB-C socket and the existing DC power adapter both face backwards.
The shell incorporates the PCB guides and adapter cradle. Three small cable ties
retain the hardware; no separate PCB carrier, feet, port plate or USB extension
is required. The mains power supply stays outside.

![Front and rear views of the exported enclosure](preview.png)

**Prototype: digital geometry checked; physical hardware fit remains unverified.**
The nominal panel dimensions come from the existing project references. The PCB,
socket positions and DC adapter dimensions below are provisional, not measured
from the photographs. Check them before printing the full shell.

| Installed print | Quantity | Size, mm | Print orientation already in STL |
| --- | --- | --- | --- |
| [frame.stl](stl/frame.stl) | 1 | 176 × 96 × 23.3 | Rear mounting ears on the bed |
| [rear.stl](stl/rear.stl) | 1 | 176 × 96 × 84.7 | Outer back face on the bed |

Assembled size: **176 × 96 × 108 mm**, excluding screw heads and connected plugs.
The panel pocket has 0.35 mm clearance on each side. Acrylic: **164 × 84 × 1 mm**,
with 1 mm corner radii. Its front recess has 0.3 mm clearance per side.

The ESP32 stands on edge so its own USB socket points directly backwards.
Its provisional 58 mm length, the panel and room for connected wiring determine
the depth. There is no six-hour print-time guarantee. The two installed parts
contain **165.5 cm³ of solid geometry**, approximately half the solid volume of
the earlier Line design with all its installed parts. This is a geometry
comparison, not a slicer filament or time prediction. Thin walls, integral
fixtures and two print jobs keep fabrication straightforward. With two suitable
printers, print the frame and shell simultaneously; elapsed printing time is the
longer job. Both also fit together on a nominal 220 × 220 mm bed with suitable
placement (186 × 212 mm including 5 mm outer brim and 10 mm part spacing).

![Internal fixtures and depth explanation](inside.png)

## Files and assembly

Use [FABLAB-FR.md](FABLAB-FR.md) for the French workshop instructions and hardware
list. [minimal-frame.zip](minimal-frame.zip) contains the complete kit, including
the editable source, two main STLs, optional fit gauges, acrylic cutting outline,
previews and validation report.

1. Check the panel fit with [fit_corner.stl](fit-tests/fit_corner.stl) and PCB
   edges with [board_gauge.stl](fit-tests/board_gauge.stl). These are test pieces,
   not installed components. Measure the DC adapter and connector positions.
2. Load the panel through the front. Attach its four rear M3 inserts to the
   integral 3 mm mounting ears, choosing screw engagement for the actual inserts.
3. Seat the acrylic from the front. Small removable adhesive strips on the ledge
   retain it. This avoids a third printed retaining ring.
4. Slide the ESP32 USB-end first into the rear shell's grooves, component face
   toward the outer side wall. Pass a 2.5 mm cable tie through the closed windows
   above its far end. Seat and strap the DC adapter with two more ties through
   the cradle's paired windows. Leave the socket mouths and terminals clear.
5. Connect the existing wiring, retain a service loop, and close the two halves
   with four M3 × 30 mm screws inserted from the front corners. Check plug access
   and connector retention before powering the display.

## Adjust and rebuild

Edit the dimensions at the top of [model.scad](model.scad). `part="assembly"`
previews the assembly; `part="exploded"` separates the shell. Other part names
are listed in the source. `reference/` contains hardware envelopes for checking,
**not additional parts to print**. Coordinates: the front is Z=0 and the back
is Z=108 in assembly view; both printable STLs are independently placed on Z=0.

Measure these parameters in particular:

| Parameter | Current assumption | Fit check |
| --- | --- | --- |
| `panel_w`, `panel_h`, `panel_depth` | 160 × 80 × 14.5 mm | Actual panel body and rear insert positions |
| `panel_clearance` | 0.35 mm per side | Sliding fit in the corner sample |
| `pcb_l`, `pcb_w`, `pcb_t` | 58 × 29 × 1.6 mm | Bare PCB; includes enough clear edge for grooves |
| `pcb_slot` | 2.2 mm | Actual board thickness and printer tolerance |
| `pcb_stop` | 2.8 mm behind the back face | Actual USB overhang and recess; assumed overhang 1.5 mm |
| `usb_open_x`, `usb_open_y` | 12 × 20 mm | Actual plug body; modeled plug envelope 10 × 14 mm |
| `power_l`, `power_w`, `power_h` | 35 × 16 × 14 mm | Existing barrel-to-terminal adapter body |
| `power_open_d` | 13 mm | Adapter nose and plug; modeled nose Ø11, plug Ø12 mm |
| `rear_z` | 108 mm | Board guides and actual connected wire clearance |

The guides engage only 0.8 mm of each long PCB edge. Check that edge for solder,
headers and components. Changes to USB position or the power adapter's shape may
require editing the corresponding mock and fixture modules as well as the values.
Panel mounting centers are fixed at 125 × 65 mm; alter the mounting ears and
validation if the actual panel differs.

With OpenSCAD available and Python dependencies from `requirements.txt`:

```sh
python3 -m pip install -r requirements.txt
python3 build.py
python3 validate.py
python3 render.py
python3 package.py
```

Run from this directory. `OPENSCAD` may specify a native executable or a Node
WASM command, e.g. `OPENSCAD="node /path/to/openscad.js"`. This export was made
using OpenSCAD 2026.10.03 (Manifold). The build removes collapsed zero-area STL
faces caused by export rounding; it does not fill holes. The manifest records
source/export hashes and the number of removed faces.

`validation.json` records connected watertight solids, bed placement, assembly
collisions, panel/PCB insertion paths, tie paths, plug and wiring envelopes, and
screw-head clearance. These checks do not establish real-world fit, strength,
thermal behavior or print duration. Regenerate and validate after any dimension
change; use the workshop's slicer profile for the actual machines.
