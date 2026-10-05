/* INGE / AFTER HOURS — dimensioned CONCEPT, not fit-approved.
   Millimetres. Set verified_dimensions only after measuring the real hardware.
   Default panel outline is evidenced by the photographed P2.5 / 80x160 moulding.
   ALL panel depth, PCB, holes, connector and fit values remain provisional.
   Parts: assembly, exploded, shell, lid, ports, carrier, clip, stand, coupon, front_spacer, acrylic_template.
   See README.md for assembly, print orientation and unresolved power topology. */

/* [Output] */
part = "assembly"; // [assembly, exploded, shell, lid, ports, carrier, clip, stand, coupon, front_spacer, acrylic_template]
verified_dimensions = false;
allow_prototype_export = false;
show_hardware = true;

/* [Panel - measure before printing] */
panel_w = 160;
panel_h = 80;
panel_depth = 15; // Rear structural edge, NOT highest cable/connector
panel_clearance = 0.4; // Per side
front_overlap = 0.6; // Check no LED is obscured

/* [Front acrylic - measure stock thickness] */
acrylic_thickness = 2;
acrylic_clearance = 0.3; // Lateral clearance per side and allowance for a soft pad
acrylic_border = 2; // Acrylic extends beyond panel outline on every edge
led_gap = 1.5; // Printed spacer between acrylic pocket and panel face
show_acrylic = true;

/* [Case] */
wall = 2.4;
front_thickness = 3;
rear_depth = 60; // Front surface to rear lid seat
lid_thickness = 3;
side_space = 8.6;
corner_radius = 4;
fit = 0.3;
screw_clearance = 3.3;
screw_pilot = 2.6; // Provisional printed pilot; pre-tap M3, fit-test first

/* [Replaceable rear connector plate - provisional hardware] */
port_plate_w = 92;
port_plate_h = 34;
port_plate_t = 3;
power_mount_d = 8; // Panel jack mounting thread, NOT barrel plug diameter
usb_open_w = 14;
usb_open_h = 7;
usb_mount_pitch = 24; // Selected USB-C panel extension flange, not USB standard
usb_mount_d = 3.3;

/* [Controller carrier - not measured] */
pcb_l = 58;
pcb_w = 29;
pcb_hole_l = 52;
pcb_hole_w = 23;
pcb_hole_d = 3.3;
pcb_standoff = 6;

/* [Hidden] */
$fn = 40;
eps = 0.05;
outer_w = panel_w + 2*(panel_clearance+side_space+wall);
outer_h = panel_h + 2*(panel_clearance+side_space+wall);
inner_w = outer_w - 2*wall;
inner_h = outer_h - 2*wall;
clip_x = panel_w/2 + panel_clearance + 4;
clip_y = panel_h/2 - 18;
acrylic_w = panel_w + 2*acrylic_border;
acrylic_h = panel_h + 2*acrylic_border;
acrylic_pocket_z = front_thickness + acrylic_thickness + acrylic_clearance;
panel_front_z = acrylic_pocket_z + led_gap;
clip_z = panel_front_z + panel_depth;
port_origin = [-32,-24];
carrier_origin = [39,11];
carrier_w = pcb_l + 22;
carrier_h = pcb_w + 16;
lid_screws = [for(x=[-1,1],y=[-1,1]) [x*(outer_w/2-5),y*(outer_h/2-5)]];
carrier_mounts = [for(x=[-1,1]) [x*(carrier_w/2-4),0]];

assert(panel_w > 100 && panel_h > 60, "Review case layout for smaller panels");
assert(front_overlap > 0 && front_overlap < 1.25, "Front lip may obscure LEDs");
assert(acrylic_thickness>=1 && acrylic_thickness<=4 && led_gap>=1,"Review acrylic thickness and LED gap");
assert(rear_depth > panel_front_z+panel_depth+pcb_standoff+18, "Insufficient rear cavity");
assert(part=="assembly" || part=="exploded" || part=="coupon" || part=="acrylic_template" || verified_dimensions || allow_prototype_export,
 "UNVERIFIED hardware: measure first or explicitly allow_prototype_export for labelled prototypes");
if (!verified_dimensions) echo("PROTOTYPE: hardware dimensions and assembled fit NOT verified");

// Small engraved pixel lettering avoids machine-dependent font substitution.
label_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ /";
label_rows = [[2, 5, 7, 5, 5], [6, 5, 6, 5, 6], [7, 4, 4, 4, 7], [6, 5, 5, 5, 6], [7, 4, 6, 4, 7], [7, 4, 6, 4, 4], [7, 4, 5, 5, 7], [5, 5, 7, 5, 5], [7, 2, 2, 2, 7], [1, 1, 1, 5, 7], [5, 5, 6, 5, 5], [4, 4, 4, 4, 7], [5, 7, 7, 5, 5], [5, 7, 7, 7, 5], [7, 5, 5, 5, 7], [7, 5, 7, 4, 4], [7, 5, 5, 7, 1], [6, 5, 6, 5, 5], [7, 4, 7, 1, 7], [7, 2, 2, 2, 2], [5, 5, 5, 5, 7], [5, 5, 5, 5, 2], [5, 5, 7, 7, 5], [5, 5, 2, 5, 5], [5, 5, 2, 2, 2], [7, 1, 2, 4, 7], [0, 0, 0, 0, 0], [1, 1, 2, 4, 4]];
function label_index(c, i=0) = i>=len(label_chars) ? -1 : label_chars[i]==c ? i : label_index(c,i+1);
module engraved_label(value, x, y, top, scale=0.65) {
    for(i=[0:len(value)-1]) let(index=label_index(value[i]))
        if(index>=0) for(row=[0:4],col=[0:2])
            if(floor(label_rows[index][row]/pow(2,2-col))%2==1)
                translate([x+(i*4+col)*scale,y+(4-row)*scale,top-0.6]) cube([scale-0.06,scale-0.06,0.7]);
}

module rounded_rect(w,h,r) {
    hull() for(x=[-1,1],y=[-1,1]) translate([x*(w/2-r),y*(h/2-r)]) circle(r=r);
}
module slab(w,h,t,r=3) { linear_extrude(t) rounded_rect(w,h,r); }
module drill(x,y,z,d,h) { translate([x,y,z]) cylinder(d=d,h=h); }
module tie_anchor(x,y) {
    // Two shoulders and top bridge create a transverse zip-tie tunnel.
    translate([x,y,27]) difference() {
        cube([12,7,5],center=true);
        cube([7,9,2.2],center=true);
    }
}
module shell() {
    difference() {
        union() {
            difference() {
                slab(outer_w,outer_h,rear_depth,corner_radius);
                translate([0,0,front_thickness]) slab(inner_w,inner_h,rear_depth,2);
                translate([0,0,-eps]) slab(panel_w-2*front_overlap,panel_h-2*front_overlap,
                    front_thickness+2*eps,0.7);
            }
            // Continuous acrylic pocket fence, tied into the front frame.
            translate([0,0,front_thickness-eps]) difference() {
                slab(acrylic_w+5,acrylic_h+5,acrylic_thickness+acrylic_clearance+led_gap+eps,2);
                translate([0,0,-eps]) slab(acrylic_w+2*acrylic_clearance,acrylic_h+2*acrylic_clearance,10,1.3);
            }
            // Front corner locating stops leave room for the panel and avoid LEDs.
            for(x=[-1,1],y=[-1,1]) translate([x*(panel_w/2+panel_clearance+1),y*(panel_h/2-7),panel_front_z])
                cube([2,10,3],center=true);
            // Retaining-clip towers join the side wall and front frame.
            for(x=[-1,1],y=[-1,1]) hull() {
                translate([x*clip_x,y*clip_y,front_thickness-eps]) cylinder(r=4,h=clip_z-front_thickness+eps);
                translate([x*(inner_w/2-1),y*clip_y,front_thickness-eps]) cylinder(r=3,h=clip_z-front_thickness+eps);
            }
            // Lid screw bosses connect to both adjacent walls.
            for(p=lid_screws) translate([p[0],p[1],rear_depth-10]) cylinder(r=4.5,h=10);
            // Lower open power channel: shelf + inner lip, outer wall closes the lane.
            // Kept clear of the upper ribbon anchors and the ventilation slots.
            translate([-64,-inner_h/2,24]) cube([128,8.5,1.5]);
            translate([-64,-inner_h/2+7,25.45]) cube([128,1.5,5]);
            // Cable guides sit at the perimeter: upper ribbon, lower power pair.
            for(x=[-48,0,48],y=[-1,1]) tie_anchor(x,y*(inner_h/2-2));
        }
        // Lens and spacer insertion pocket also clears the inner edges of clip towers.
        translate([0,0,front_thickness]) slab(acrylic_w+2*acrylic_clearance,acrylic_h+2*acrylic_clearance,acrylic_thickness+acrylic_clearance+led_gap,1.3);
        for(x=[-1,1],y=[-1,1]) drill(x*clip_x,y*clip_y,clip_z-10,screw_pilot,12);
        for(p=lid_screws) drill(p[0],p[1],rear_depth-9,screw_pilot,11);
        // Aligned top/bottom slots, away from corner fasteners and tie anchors.
        for(x=[-66,-60,-54,-36,-30,-24,18,24,30,54,60,66],y=[-1,1])
            translate([x,y*outer_h/2,43]) cube([3,wall*3,18],center=true);
    }
}
module front_spacer() {
    // Broad edge support; no pressure on individual LEDs. Fit-check border first.
    difference() {
        slab(acrylic_w,acrylic_h,led_gap,1);
        translate([0,0,-eps]) slab(panel_w-2*front_overlap,panel_h-2*front_overlap,led_gap+2*eps,0.7);
    }
}
module acrylic_template() { rounded_rect(acrylic_w,acrylic_h,1); }
module acrylic_sheet() { slab(acrylic_w,acrylic_h,acrylic_thickness,1); }
module panel_clip() {
    difference() {
        translate([-3,0,0]) slab(16,8,2.4,1.4);
        drill(2,0,-eps,screw_clearance,3);
    }
}
module lid() {
    difference() {
        slab(outer_w,outer_h,lid_thickness,corner_radius);
        for(p=lid_screws) drill(p[0],p[1],-eps,screw_clearance,lid_thickness+2*eps);
        translate([port_origin[0],port_origin[1],-eps]) slab(76,20,lid_thickness+2*eps,2);
        for(s=[-1,1]) drill(port_origin[0]+s*(port_plate_w/2-5),port_origin[1],-eps,screw_clearance,lid_thickness+2*eps);
        for(p=carrier_mounts) drill(carrier_origin[0]+p[0],carrier_origin[1]+p[1],-eps,screw_clearance,lid_thickness+2*eps);
        engraved_label("INGE / AFTER HOURS",5,39,lid_thickness,0.65);
        // Exhaust section deliberately clear of the PCB and rear connector plate.
        for(x=[-70:6:-12]) translate([x,20,-eps]) slab(3,24,lid_thickness+2*eps,1.4);
    }
}
module ports() {
    difference() {
        slab(port_plate_w,port_plate_h,port_plate_t,3);
        for(s=[-1,1]) drill(s*(port_plate_w/2-5),0,-eps,screw_clearance,port_plate_t+2*eps);
        drill(-22,0,-eps,power_mount_d,port_plate_t+2*eps);
        translate([17,0,-eps]) slab(usb_open_w,usb_open_h,port_plate_t+2*eps,1.4);
        for(s=[-1,1]) drill(17+s*usb_mount_pitch/2,0,-eps,usb_mount_d,port_plate_t+2*eps);
        engraved_label("POWER",-28,10,port_plate_t);
        engraved_label("USB",13,10,port_plate_t);
    }
}
module carrier() {
    difference() {
        union() {
            slab(carrier_w,carrier_h,2.4,3);
            for(x=[-1,1],y=[-1,1]) translate([x*pcb_hole_l/2,y*pcb_hole_w/2,2.4-eps])
                cylinder(d=6.5,h=pcb_standoff+eps);
        }
        for(x=[-1,1],y=[-1,1]) drill(x*pcb_hole_l/2,y*pcb_hole_w/2,-eps,pcb_hole_d,12);
        for(p=carrier_mounts) drill(p[0],p[1],-eps,screw_clearance,4);
        // The carrier mounting nuts remain accessible outside the board outline.
        for(x=[-18,0,18],y=[-1,1]) translate([x,y*(carrier_h/2-4),-eps])
            slab(8,2.4,3,0.9);
    }
}
module coupon() {
    // Printable test of port apertures, M3 clearances and pilot sizes.
    difference() {
        slab(92,42,3,3);
        drill(-28,2,-eps,power_mount_d,4);
        translate([5,3,-eps]) slab(usb_open_w,usb_open_h,4,1.4);
        for(i=[0:3]) drill(-28+i*17,-12,-eps,2.6+i*0.2,4);
    }
}
module stand() {
    // Print twice. A 10-degree wedge with a front lip; add non-slip pads.
    d=rear_depth+lid_thickness+12;
    union() {
        translate([-15,0,0]) rotate([90,0,90]) linear_extrude(30)
            polygon([[0,0],[d,0],[d,3+d*tan(10)],[0,3]]);
        translate([-15,0,2.5]) cube([30,3,8]);
    }
}
module hardware() {
    color([0.04,0.05,0.06]) translate([0,0,panel_front_z+0.5]) slab(panel_w,panel_h,panel_depth-0.5,0.8);
    // Illustrative board envelope only: connectors/wires omitted, fit not validated.
    color([0.02,0.3,0.18]) translate([carrier_origin[0],carrier_origin[1],rear_depth-2.4-pcb_standoff-1.6])
        slab(pcb_l,pcb_w,1.6,1.5);
}
module assembly(explode=false) {
    color([0.13,0.14,0.17]) shell();
    if(show_acrylic) color([0.55,0.85,0.95,0.28]) translate([0,0,front_thickness-(explode?35:0)]) acrylic_sheet();
    color([0.18,0.21,0.23]) translate([0,0,acrylic_pocket_z-(explode?17:0)]) front_spacer();
    for(x=[-1,1],y=[-1,1])
        color([0.52,0.66,0.69]) translate([x*clip_x,y*clip_y,clip_z+(explode?8:0)])
            rotate([0,0,x==1?0:180]) translate([-2,0,0]) panel_clip();
    color([0.14,0.16,0.18]) translate([0,0,rear_depth+(explode?40:0)]) lid();
    color([0.6,0.8,0.75]) translate([port_origin[0],port_origin[1],rear_depth+lid_thickness+(explode?65:0)]) ports();
    color([0.36,0.42,0.45]) translate([carrier_origin[0],carrier_origin[1],rear_depth+(explode?24:0)])
        rotate([180,0,0]) carrier();
    if(show_hardware && !explode) hardware();
}
if(part=="shell") shell();
else if(part=="lid") lid();
else if(part=="ports") ports();
else if(part=="carrier") carrier();
else if(part=="clip") panel_clip();
else if(part=="coupon") coupon();
else if(part=="front_spacer") front_spacer();
else if(part=="acrylic_template") acrylic_template();
else if(part=="stand") stand();
else if(part=="exploded") assembly(true);
else if(part=="assembly") assembly(false);
else assert(false,"Unknown part");
