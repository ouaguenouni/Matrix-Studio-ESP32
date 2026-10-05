# Inge — gift enclosure design brief

Status, 5 October 2026: the five digital enclosure kits are completed in
[`cad/enclosure-comparison`](../cad/enclosure-comparison/README.md), with an
offline viewer, per-design STL archives, assembly guides and fresh geometry
checks. The confirmed enclosed components are the panel, ESP32, acrylic and
wiring; the power adapter remains external. **Hardware fit remains unverified.**
The comparison family uses 1 mm acrylic and existing cable pass-throughs.
[`cad/inge-after-hours`](../cad/inge-after-hours/README.md) is the older single-case
prototype and the research notes below describe that earlier proposal. The user
confirmed replacing the picture frame with the complete printed case.

All five supplied photos have been inspected. The moulded panel marking reads
P2.5-1 / 80×160; the board has USB-C and four corner holes. The external adapter
label appears 5 V / 3 A / 15 W and needs physical confirmation. No ruler or hole
measurements were supplied. USB-C flange, power-inlet mount, PCB and rear-depth
parameters therefore remain provisional. The prototype uses a short data-capable
USB-C extension, a proposed extra component requiring selection before final fit.

Artilect is the recommended Toulouse venue for the 9 October birthday deadline,
based on posted public opening times and guided printing access. Its official
220×220 mm Ender 3 bed accommodates the prototype's 182.8×102.8 mm footprint.
CampusFab lists Bambu machines but its fresh homepage showed no forthcoming
opening session during research; neither venue has been booked or contacted.
See the CAD README for citations, print workflow and remaining measurements.

The original research notes below remain useful design context; where they differ,
the prototype README and measured parameters take precedence.

## Direction

A small architectural desk object, provisionally labelled **INGE / AFTER HOURS**:
a clean dark frame, recessed LED face, fine parallel ventilation ribs, and a
single optional coloured accent. Keep the inscription on a replaceable rear
nameplate so the front stays quiet. A separate shallow wedge stand provides a
comfortable viewing angle without complicating the main enclosure print.

This is a proposal, not an assumption about Inge's preferred colours or sensory
preferences. Confirm the visual direction before finishing decorative details.

## What the repository establishes

- One Waveshare 64×32 HUB75/FM6124 panel and a classic ESP32-WROOM development
  board. Neither the exact panel pitch nor the development-board PCB is known.
- The panel uses an external regulated 5 V supply. The README explicitly keeps
  the ESP32 external 5 V pin disconnected while it is powered from USB; grounds
  are shared. A USB-C socket on the actual development board is not confirmed.
- BOOT changes display mode; BOOT and EN/RST can also be needed for servicing.
- No physical dimension drawings, enclosure CAD, or component photographs were
  found among the repository's tracked/project files inspected for this brief.

64×32 is a pixel count, not a case size. Waveshare lists P2.5, P3, P4, and P5
variants with respective face dimensions 160×80, 192×96, 256×128, and 320×160 mm.
Its current specification lists 5 V / 2.5 A for P2.5/P3 and 5 V / 4 A for P4/P5.
Confirm the actual SKU; these are not measurements of this project's panel.

## Mechanical architecture

1. **Front frame and shell:** retain the panel using its actual mounting bosses
   or holes, with insulating spacers and no force against the LEDs or solder
   joints. Avoid covering the outermost pixels. Use a shallow protective rim;
   test any optional smoked acrylic separately for readability and reflections.
2. **Removable rear lid:** screw attachment for repeatable access. Reserve room
   for actual screw lengths and component clearances. Start with captive nuts;
   insert pockets can be substituted once the user's fastener stock is known.
3. **Removable controller carrier:** mount the real board, keeping its antenna
   away from the panel's metal backing and bundled conductors. Provide internal
   BOOT/EN access and, if needed, a non-preloaded external BOOT plunger.
4. **Replaceable rear connector plate:** exactly two functional cable openings,
   labelled `5 V POWER` and `USB / FLASH`. Size power opening for the selected
   inlet or strain-relieved captive cable. Prefer direct access to the actual
   board USB socket if it is USB-C and placement permits; otherwise reserve a
   mount for a verified USB data-capable panel extension, not a charging cable.
5. **Cable routing:** separate perimeter channels for the power pair and HUB75
   ribbon, tie-down slots, rounded exits, and service loops at removable parts.
   Keep the ribbon fold gentle; do not trap cables beneath the lid or over vents.
   Cable strain relief attaches to the shell, not the board connector.
6. **Passive cooling:** lower intake and upper rear exhaust slots, with stand
   clearance underneath. Keep slots away from fastening bosses and electrical
   connections. Check enclosed temperature under sustained real usage before
   calling the design complete.

Initial CAD starting values, to validate with fit coupons: 2.4 mm walls, 3 mm
lid, approximately 0.3 mm mating clearance per side, four M3 lid fasteners,
and a 10-degree removable wedge. These are design targets, not measured fit or
print specifications. Case depth follows the deepest connected component plus
its actual cable bend, mounting, and airflow clearance; do not assume 40 mm is
enough. Large variants may need a split shell to fit the printer bed.

## Power arrangement to resolve before construction

Default proposal: keep the mains adapter outside the gift enclosure; only
regulated low-voltage wiring enters it. The power inlet must be rated for the
confirmed panel plus controller load and use the confirmed polarity.

A two-port case does not establish a safe single-cable everyday power circuit.
If the power inlet is to feed both panel and ESP32, inspect the actual board's
power schematic and choose a supported supply-selection/isolation arrangement
before connecting computer USB at the same time. Preserve the README's existing
restriction until this is verified. Do not simply join external 5 V and USB VBUS.
Allow carrier space for any required distribution or isolation module only after
its actual dimensions and wiring have been specified.

## Editable CAD and print workflow

Use a parameterised OpenSCAD source with separate shell, lid, connector plate,
controller carrier, and stand modules. Inputs should include the measured panel
outline and mounting-hole coordinates, board outline and hole coordinates,
connector envelopes, cable clearance, printer bed, wall thickness, mating
clearance, and fastener dimensions. Use explicit unknowns rather than plausible
defaults for the component dimensions.

After measurements: produce the editable `.scad`, per-part STL files, an assembly
view, dimensions/fastener list, and a brief assembly guide. Validate manifold
exports, slicer size/orientation, interference and cable access. Print a mounting
corner and connector plate first; correct fits before printing the full shell.
PETG is a reasonable starting material for the functional enclosure; use the
printer/filament manufacturer's profile and test the finish. Material choice
alone does not validate the enclosure's thermal behaviour.

Final fit checks: LEDs unobstructed, panel securely retained, plugged-in cables
clear lid, no pinched wires, inlet strain relief holds, USB upload works with the
approved power procedure, BOOT remains usable, Wi-Fi stays reliable, and the
closed case stays within component/material limits during sustained operation.

## Measurements and choices still needed

- Exact panel SKU; front/back photos; width, height, rear depth; hole/boss
  positions relative to one corner; connector locations and plugged-in depth.
- Exact ESP32 board or clear top/bottom photos; PCB width/length, mounting holes,
  USB connector type/location, and maximum height with its wiring attached.
- Power adapter output rating, existing plug/connector and polarity, and whether
  the single power inlet should supply both panel and controller in daily use.
- Every other part to enclose, with dimensions: e.g. distribution board, audio
  board, speaker or another project. Their presence is **not** assumed.
- Printer model/build volume, material available, and fastener preference.
- Preferred look and stand/wall orientation; desired physical nameplate wording.

## Primary references

- [Waveshare panel family specifications](https://docs.waveshare.com/RGB-Matrix-Px-64x32)
- [Waveshare resources and official 2D drawing link](https://docs.waveshare.com/RGB-Matrix-Px-64x32/Resources-And-Documents)
  — follow the drawing matching the confirmed SKU; dimensions have not been
  transferred from a drawing into CAD yet.
- [OpenSCAD documentation](https://openscad.org/documentation)
- [OpenSCAD export and customisation manual](https://files.openscad.org/documentation/manual/OpenSCAD_User_Manual.html)
- [Prusa PETG material guidance](https://help.prusa3d.com/article/petg_2059?product=mmu3)
