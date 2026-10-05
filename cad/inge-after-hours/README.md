# INGE / AFTER HOURS — enclosure prototype

**Dimensioned, editable concept. Do not print the complete gift enclosure yet.**
The STL files are labelled prototypes because the photographs establish the panel
family and USB connector, but do not provide the mechanical measurements needed
for a reliable fit. Print the small `coupon.stl` first after selecting connectors. The bought QWORK acrylic
sheets are 180 × 130 mm; a 164 × 84 mm window must be cut from one sheet.

## Files

- `enclosure.scad`: editable parametric source; choose `part` in OpenSCAD Customizer.
- `prototype-stl/`: individual printable meshes for review/fit testing, not approved hardware fit.
- `acrylic-front-164x84.svg`: full-size cutting outline for the purchased acrylic.
- `validate_and_render.py`: checks STL topology/size and renders the actual meshes.
- `validation.json` / `prototype-overview.png`: geometry results and exploded view.
- `assembled-preview.png`: front/rear assembled views; panel face is illustrative.
- `FABLAB-FR.md`: French handoff sheet for the workshop, including fit checks.

The user confirmed that the complete printed enclosure replaces the picture frame.
The rear cover carries an engraved INGE / AFTER HOURS inscription; the removable
connector plate is engraved POWER and USB.

## Evidence from the five photographs

All five `pictures/IMG_20261005_*.jpg` were inspected. The rear moulding reads
**P2.5-1 / 80×160**, consistent with Waveshare's P2.5 panel dimensions. The user identifies the controller as **ESP32 NodeMCU WiFi Bluetooth Module
ESP32 WROOM 32 Development Board**. This family name does not specify a PCB
revision or mechanical drawing. The photographed ESP32
has a **USB-C** socket, four PCB corner holes, and BOOT/EN buttons. The photos show
an external wall adapter and barrel-jack-to-terminal-block adapter. Its oblique
label appears **5.0 V / 3.0 A / 15.0 W**; verify on the actual label. The barrel's
inner/outer diameter and polarity cannot be determined from these photos.

No ruler, depth measurement, exact board model, hole centres or connector drawings
are visible. A PCB scale inferred from fingers or perspective is not reliable.
The official panel drawing is available as DWG from Waveshare, but was not parsed
into this model; the panel retention scheme therefore avoids guessing its holes.

## Parts and mechanical layout

| Part | Quantity | Purpose |
| --- | ---: | --- |
| shell | 1 | Recessed front rim, panel seat, clip towers, lid bosses, cable anchors and vents |
| front_spacer | 1 | Separates the acrylic pocket from the LED face; printed flat |
| acrylic (not printed) | 1 | 164 × 84 mm laser-cut window from the purchased QWORK sheet |
| lid | 1 | Removable rear cover; separate vents, carrier mount and connector-plate opening |
| ports | 1 | Replaceable plate for power jack and a USB-C data extension |
| carrier | 1 | Controller tray with stand-offs; board dimensions/hole spacing provisional |
| clip | 4 | Retain panel structural edges without using unmeasured panel holes |
| stand | 2 | 10-degree desk wedges; add non-slip pads and check stability with connected cables |
| coupon | 1 | Power/USB cutout and 2.6/2.8/3.0/3.2 mm hole fit tests |

Default body: **182.8 × 102.8 × 60 mm**, plus 3 mm rear lid and local 3 mm port
plate. This leaves generous rear wiring room; reduce depth only after checking
plugged-in connectors and ribbon bends. Front overlap is 0.6 mm per edge: verify
it misses every LED. Panel clips touch only the structural rear edge and should
not load the PCB or solder joints. Rear display depth defaults to 15 mm solely
for modelling; this is unverified.

The controller is carried on the inside of the lid, with component side facing
into the enclosure. Add four board fasteners with suitable nuts, and two carrier
fasteners/nuts through the lid. The USB socket is reached through a short,
**data-capable** panel-mount USB-C extension. This is a proposed additional part,
not something shown in the photographs. Select its actual flange drawing and
cable bend before printing the port plate. The provisional 8 mm power cutout is
a panel jack's *mounting* diameter, not the existing barrel plug's diameter.

The lower open channel is reserved for the power pair. Its shelf is 128 mm long,
with an inner retaining lip; the six original tie anchors remain. Route the HUB75
ribbon along the upper perimeter without sharp folds and use the upper anchors
for cable ties, rather than trying to squeeze the ribbon into the power channel.
Keep the antenna clear and leave a service loop to remove the rear lid. Tie tunnels are
small bridges; check the slicer bridge preview. Screw bosses have 2.6 mm pilot
holes as a starting point for pre-tapping M3 after coupon testing. These are not
heat-set-insert pockets. Do not force a screw into a printed undersized hole.

BOOT/EN remain accessible by removing the lid; there is no external plunger in
this prototype. If direct daily BOOT access is required, measure the switch
locations and add a separate non-preloaded plunger before the final print.

## Measurements to replace before final export

`panel_depth`, `pcb_l`, `pcb_w`, `pcb_hole_l`, `pcb_hole_w`, `pcb_hole_d`,
`pcb_standoff`, `power_mount_d`, `usb_open_w`, `usb_open_h`, `usb_mount_pitch`,
`usb_mount_d` and the case depth/cable-clearance assumptions need physical checks.
Confirm 160×80 mm panel outline, its edge-retention surfaces and plugged-in depth.

The selected printer/filament may require different mating/fastener allowances.
Parameters `fit` and `panel_clearance` document separate purposes: the current
cover is flat screw-on and does not use a friction-fit mating lip, so `fit` is
reserved for a future locating lip. No shrink compensation is hidden in the STLs.

## Printing and assembly

Start with a **0.4 mm nozzle, 0.2 mm layers, four perimeters, 15–20% infill**, PETG
and the fablab's validated machine/filament profile. These are suggested settings,
not a sliced/bench-tested profile. Print shell front-down, lid/plate flat,
carrier base-down, clips flat, and wedges base-down. The shell front-down has a
continuous frame contact ring; add a brim if the fablab recommends it. Inspect
slicer supports under elevated screw bosses/anchor bridges; local support may
be needed. Do not send machine-specific G-code until the machine is confirmed.

1. Fit-test the coupon with real connectors and screws; revise dimensions.
2. Test board carrier and one clip/seat region with real hardware.
3. Print final parts only after confirming fit. Retain the panel gently with
   four clips; do not exceed the panel's measured structural height.
4. Secure connector bodies, strain relief and board; route cables through
   anchors with a service loop. Insulate all exposed electrical connections.
5. Follow the confirmed power procedure below; check USB upload with lid open.
6. Close the lid, check Wi-Fi/controls, cable pulls and desk stability, and test
   temperature under sustained usage before wrapping the gift.

Fastener lengths depend on actual panel depth, PCB and connector flange thickness.
Choose lengths at assembly to prevent bottoming out or contacting electronics;
no unverified universal screw-length shopping list is provided.

## Power is a separate unresolved design decision

Keep the mains wall adapter external. The existing README says external ESP32
5 V must stay disconnected during USB powering. The photographs show attached
wires but do not establish a protected power path. **Do not assume simultaneous
external 5 V and computer USB are safe.** If one everyday power inlet must feed
both panel and ESP32, identify the exact board and have its supply selection or
isolation arrangement verified. The enclosure openings do not solve this circuit.

## Export/reproduce

OpenSCAD desktop can render each part and export STL. For scripted generation:

```sh
openscad --export-format binstl -o prototype-stl/shell.stl \
  -D 'part="shell"' -D allow_prototype_export=true enclosure.scad
```

`allow_prototype_export=true` is an explicit acknowledgement of unknown dimensions.
Set `verified_dimensions=true` only after measuring and fit-testing; changing the
flag itself does not verify a design. Other parts use the same command and their
part names. `assembly` and `exploded` are review views, not single printable pieces.

Validation/render dependencies are `trimesh`, `scipy` and `matplotlib`:

```sh
python3 validate_and_render.py
```

Meshes were exported using official OpenSCAD 2026.10.03 WebAssembly Node build
in `/tmp`; no CAD application was installed system-wide. Geometry checks cannot
prove dimensional fit, material behaviour, electrical suitability or print quality.

## Toulouse fablab research — 5 October 2026

**Recommendation: Artilect**, prioritising guided access before Inge's 9 October
birthday. Its posted public schedule includes Tuesday–Friday 08:30–19:00 and
Saturday 10:00–18:00. Machine brand alone cannot establish finish quality; request
a calibrated PETG profile and inspect a small sample first. **Artilect** officially lists
Ender 3 machines with 220×220×250 mm beds, PETG support, and a CR-10 S5 with a
500 mm cube. The current enclosure fits a 220 mm bed with a modest brim; printing
the large parts separately avoids overfilling the bed. Artilect asks users to
start with an initiation and book machines. Confirm a slot before the birthday;
no reservation has been made.

**RoseLab** confirms a 3D lab but its public lab page did not identify exact
available machines/usable build volume during this review. **CampusFab** lists
Bambu P1S and X1C machines and requires training before autonomous access. Confirm
current machine availability and staff-selected slicing settings. CampusFab's
fresh homepage did not display a forthcoming opening session during this check;
its indexed machine list is not confirmation of bookable access before 9 October.
No venue was contacted and no booking or payment has been made.

## Sources

- [Waveshare specifications](https://www.waveshare.com/RGB-Matrix-P2.5-64x32.htm)
- [Official P2.5 DWG](https://files.waveshare.com/wiki/RGB-Matrix-P2.5-64x32/RGB-Matrix-P2.5-64x32-2D.dwg)
- [OpenSCAD official downloads](https://openscad.org/downloads.html)
- [Artilect machines and printing process](https://www.artilect.fr/impression-3d)
- [RoseLab labs](https://roselab.eu/les-labs/)
- [CampusFab equipment and access](https://campusfab.net/)

## Supplied product links

- Panel: https://www.amazon.fr/dp/B0BQYDLHY9 — supplied by the user; matches the
  photographed Waveshare P2.5 family. The 160 × 80 mm outline is supported by the
  manufacturer specification and the moulded panel marking.
- Controller: https://www.amazon.fr/dp/B0DB8F8MKH?th=1 — supplied by the user.
  User-provided product title: **ESP32 NodeMCU WiFi Bluetooth Module ESP32 WROOM 32 Development Board**.
  Listing contents could not be verified; browser access was denied. USB-C and
  four mounting holes are confirmed by the actual board photograph, but its
  precise outline and hole coordinates remain provisional.

The editable model deliberately does not claim measured fit from an ASIN alone.
The workshop can complete the remaining measurements and fit tests with the
physical hardware, using FABLAB-FR.md; the user need not guess these dimensions.

## Acrylic front — purchased QWORK sheets

User-supplied product: **QWORK 10 × Transparent Acrylic Sheets, 13 × 18 cm**.
Sheet stock is 180 × 130 mm; the CAD window is **164 × 84 mm**, with 1 mm corner
radii. `acrylic-front-164x84.svg` is a 1:1 millimetre laser-cut outline. Cut the
sheet; it does not fit this compact case at its full purchased size. Confirm
the material from the packaging before the workshop chooses its cutting process.
The SVG page includes a 1 mm margin: its page is 166 × 86 mm, but the actual
cut path is 164 × 84 mm. Import at 100%; do not resize the page to the cut size.

Thickness was not given in the product title. `acrylic_thickness=2` is a **CAD
starting assumption**, not a claim about these sheets. Measure the stock at the
workshop and regenerate the shell before final printing. `acrylic_clearance=0.3`
provides per-side fit allowance and a thickness allowance for a thin compliant
edge pad; select/test the actual pad with the actual sheet so it cannot rattle.

Front-to-back stack: printed front rim → acrylic sheet in a locating pocket →
printed perimeter spacer → LED panel → retaining clips. The nominal 1.5 mm spacer
keeps the sheet away from the LEDs. Check actual LED height and border clearance
with the real panel; adjust `led_gap` if needed. Remove the panel/retainers from
the rear to replace the acrylic; no permanent glue is required. Clips must bear
on the panel's structural frame and must not over-compress the acrylic.

The clear sheet protects the front; optical diffusion is not assumed. The preview
uses an illustrative tint to show its location, not a prediction of transmission.
Power and ribbon routes leave a service loop for opening the lid. The external
connector bodies must be firmly mounted; cable ties provide internal retention
but are not a certified cable gland or a substitute for a proper panel connector.
