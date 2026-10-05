# Print and assembly guide — five enclosure kits

Dimensions and STL units are millimetres. This is a complete digital prototype
handoff; hardware fit, temperature and cable-pull checks require a real assembly.
Use one variant's files throughout. Each ZIP preserves the shared source beside
its variant folder, so `model.scad` can find `../family.scad` after extraction.

## Components and nominal space

- Waveshare P2.5 panel: nominal 160 × 80 × 14.5 mm, four outer rear M3 inserts at
  125 × 65 mm spacing. Source drawing evidence is in `evidence/`; confirm your
  panel's inserts and permissible screw engagement at the workshop.
- ESP32: provisional PCB 58 × 29 × 1.6 mm, nominal holes 52 × 23 mm apart. Rails
  slide on the carrier and PCB holes slide along the rails. Measure the board;
  the photograph proves neither these dimensions nor an M3-compatible hole size.
- Acrylic: 1 mm thick. Gallery/Vault accept 180 × 130 mm sheets; the other designs
  require the supplied 164 × 84 mm SVG cut outline, imported at 100% scale.
  SVG pages include a 1 mm margin all around (166 × 86 or 182 × 132 mm).
  The cut path has the stated sheet dimensions; do not resize the page to them.
- Existing power and USB leads pass through 22 × 14 mm and 32 × 18 mm rear
  openings. Check actual plug envelopes. Cable straps attach to the plate's slots.
- The panel gets a nominal 20 mm plugged-in connector reserve behind its rear
  face, apart from the mounting bridges. The controller bay reserves 64 × 36 ×
  38 mm, including underside jumpers. These are explicit allowances, not measured
  representations of connectors. Check every connector with its cable plugged in.

## Parts to print for ONE enclosure

| File in the chosen `stl/` folder | Quantity | Purpose and orientation |
| --- | ---: | --- |
| `bezel.stl` | 1 | Front face down; separately removable acrylic holder |
| `keeper.stl` | 1 | Flat; acrylic retaining ring |
| `body.stl` | 1 | Supplied Z=0 face down; open through shell |
| `lid.stl` | 1 | Flat; removable rear cover |
| `bridge.stl` | 2 | Flat; mounts panel by rear inserts |
| `carrier.stl` | 1 | Base down; adjustable ESP32 mounting frame |
| `rail.stl` | 2 | Flat; adjusts longitudinal PCB hole spacing |
| `spacer.stl` | 4 | Upright; 20 mm default wiring clearance under PCB |
| `cassette.stl` | 1 | Base down; removable oval cable guide |
| `ports.stl` | 1 | Flat; pass-through plate for existing cables |
| `feet.stl` | 2 | Supplied flat orientation; rotate when assembling |
| `coupon.stl` | 1 | Print FIRST; screw/acrylic/round-inlet fit gauges |
| `ports_usb_c.stl` | 0 by default | Optional substitute for `ports`, never an extra plate |

There are 17 installed printed pieces and one test coupon. Duplicate bridges,
rails, spacers and feet in the slicer according to the table; one STL is one piece.
The holes in the coupon run left-to-right 1.6, 1.8, 2.0, 2.2, 2.4, 2.6, 2.8, 3.0 and 3.4 mm.
The three open acrylic grooves are 1.0, 1.2 and 1.4 mm. The large round openings
are 12.2, 12.4 and 12.6 mm, intended only for the optional round USB adapter.
Check the rectangular plate openings with a test print of the actual plate.

## Hardware to bring

| Use | Quantity | Nominal fastener |
| --- | ---: | --- |
| Bezel to shell | 4 | M3 screw into prepared printed 2.6 mm pilot |
| Lid to shell | 4 | M3 screw into prepared printed 2.6 mm pilot |
| Bridges to shell seats | 4 | M3 screw into prepared printed 2.6 mm pilot |
| Panel to bridges | 4 | M3 screw into the panel's existing inserts |
| Acrylic keeper to bezel | 4 | M2 screw into prepared printed 1.6 mm pilot |
| Carrier to lid | 2 | M3 bolt + nut |
| Cassette to lid | 2 | M3 bolt + nut |
| Port plate to lid | 2 | M3 bolt + nut |
| Rails to carrier | 4 | M3 bolt + nut; nuts in carrier channels |
| PCB/spacers to rails | 4 | Bolt + nut matched to actual board holes, maximum M3 |

For the default M3-compatible PCB: 30 M3 screws/bolts, 14 M3 nuts and 4 M2 screws.
Add appropriate washers, soft cable straps/ties, non-slip pads and removable tape
for the cradle feet. Screw lengths are selected against real material stacks:
panel bolts cross a 3 mm bridge before entering the inserts; board bolts cross
PCB thickness + 20 mm spacer + 3 mm rail plus washer/nut allowance. Lid-mounted
parts have 3 mm lid + 3 mm plate/frame bases (cassette base: 2.4 mm). Check nuts
and washers clear wires and vents. Pilot bosses allow about 12 mm engagement at
front/rear, 9 mm at bridge seats; do not bottom screws or reach the electronics.
M2 keeper pilots begin 1 mm behind the front; leave that front wall intact.

The carrier channels are 6.1 mm wide and 2.5 mm deep. Measure your nuts; thicker
nuts need a revised channel. Four 5.6 × 2.4 mm M3 head envelopes are checked at
the panel mounts, bridge mounts, front, rear and PCB. Keeper heads are checked
at 5.5 × 2.5 mm; they must fit the shell reliefs. Other nut/head clearances need
a fit assembly. Pre-tap suitable printed pilots after coupon testing; do not
force metric screws into undersized printed holes.

## Print workflow

The largest footprint is 206 × 156 mm. All supplied orientations leave at least
5 mm brim space on an assumed 220 × 220 mm bed and stay below 250 mm height.
Use the workshop's validated printer/material profile. A 0.4 mm nozzle, 0.2 mm
layers, four perimeters and 15–20% infill are starting preferences. The included
`diagnostic-slicer.ini` is an inherited diagnostic reference, not a proven printer
profile; no slicing result or machine-ready G-code is claimed for this revision.

1. Print the coupon, carrier, one rail, one spacer and the default port plate.
   Confirm the screws, board hole size, nuts, acrylic thickness and plugged cables.
2. Confirm panel dimensions and mounting geometry against the real panel.
   Adjust CAD and regenerate if necessary; do not trim structural parts to force fit.
3. Slice the selected kit; inspect bridges over vents, nut channels and grooves.
   The shell has full-height corner bosses; vent roofs span only 3 mm. Local
   bridge/support choices depend on your printer and profile.
4. Print the remaining parts in the table, then clean and deburr the mating faces.

## Assembly order

1. Lay bezel front-down on a protected surface. Insert acrylic from the rear and
   fit the keeper with four M2 screws. The 0.3 mm thickness allowance can take a
   thin compliant edge pad after testing; the sheet should not rattle or bow.
2. Attach bezel to body using the four front M3 screws. The panel is held by its
   rear inserts, not compressed against acrylic. Nominal sheet-to-panel gap is
   4.5 mm; keeper-to-panel gap is 2.2 mm.
3. Attach the two bridges to the panel's outer rear inserts, then fasten their
   ends to the shell seats. Select panel screw lengths by permissible insert
   engagement. Leave all LED edges and solder joints unloaded.
4. Put nuts into the carrier channels. Fasten carrier to inside of lid, and fit
   the two rails against its raised pads. Slide rails to the measured PCB hole
   spacing, then tighten. Mount the board on four spacers with component side
   toward the lid; underside GPIO wiring faces the panel. See the exploded view.
5. Attach cassette and pass-through plate to lid. The plate is outside, covering
   the lid's broad opening; its cable openings face the outside. Retain the
   power/USB leads with soft straps through the adjacent slots. Avoid sharp edges
   and don't transfer cable pulls to the board's USB socket.
6. Route HUB75/jumpers in the middle compartment and power/service tails toward
   the rear plate. Gently loop spare cable around the cassette's oval guide and
   strap it loosely. The nominal guide's minimum bend radius is 25 mm, but each
   cable's actual bend requirement takes precedence. Do not tightly wind the
   HUB75 ribbon; leave its natural gentle bend and a service loop to open the lid.
7. Keep the ESP32 antenna away from dense cable bundles; inspect all clearances
   with cables plugged in. BOOT and EN are reached by removing the four rear
   screws; an external button actuator is not included.
8. Close the lid with four M3 screws. Fit two cradle feet near the side edges,
   their small lip at the front and large lip at the rear. Add pads and removable
   tape; verify stability with all cables attached.
9. Test display edges, USB upload, Wi-Fi, BOOT/EN access, cable retention, and
   temperature during sustained use. Follow the project's confirmed power wiring:
   keep external ESP32 5 V disconnected while powered from computer USB.

Bring the actual components to the workshop. Digital collision checks cannot
measure your board, determine permissible insert depth or prove thermal behaviour.
