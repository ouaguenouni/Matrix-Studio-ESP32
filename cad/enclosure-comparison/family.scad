/* INGE / AFTER HOURS, revision 3. All dimensions in mm.
   Five printable comparison prototypes; hardware fit and thermal tests pending.
   See PRINT-GUIDE.md. Panel mounts from Waveshare DWG: 125 x 65, M3.
   Print parts individually, never print assembly/exploded output. */
variant = 1; // [1:Line,2:Orbit,3:Gallery,4:Vault,5:Console]
part = "assembly"; // [assembly,exploded,body,bezel,lid,keeper,bridge,carrier,rail,spacer,ports,ports_usb_c,cassette,feet,coupon,acrylic_template]
acrylic_thickness = 1; // Confirmed by owner, 2026-10-05
panel_depth = 14.5; // Waveshare official dimension image
pilot = 2.6; // Pre-tap M3 or tune with coupon; do not force screws
keeper_pilot = 1.6; // M2 pilot; tune after coupon testing
clearance = 3.4;
// Provisional PCB envelope and adjustable hole pattern: measure the actual board.
pcb_length = 58;
pcb_width = 29;
pcb_thickness = 1.6;
pcb_hole_length = 52;
pcb_hole_width = 23;
pcb_standoff = 20;
$fn=40;
eps=0.04;
// width,height,body depth,corner radius,lens width,lens height,cable tray height
configs = [[190,110,82,5,164,84,14], [202,122,84,16,164,84,20],
           [206,156,84,8,180,130,20], [206,156,110,8,180,130,40],
           [202,122,96,7,164,84,30]];
style=is_undef(style_id) ? variant : style_id;
c=configs[style-1];
W=c[0]; H=c[1]; D=c[2]; R=c[3]; LW=c[4]; LH=c[5]; CH=c[6];
wall=3; bezel_t=6.5; lid_t=3;
// Clear opening exceeds entire nominal panel outline: no LED-edge loading.
window_w=161; window_h=81;
panel_front=8.5; panel_back=panel_front+panel_depth;
bridge_x=W/2-8;
corner_x=W/2-7; corner_y=H/2-7;
keeper_w=LW+10; keeper_h=LH+10;
keeper_z=3+acrylic_thickness+0.3;
ports_y=-H/2+21;
carrier_xy=[43,12]; cassette_xy=[-44,H>=150?36:12];
// Cassette nominal footprint78x72; split from right-hand PCB zone by3mm.
assert(style>=1 && style<=5);
assert(acrylic_thickness>0 && acrylic_thickness<=1.2,"Recess needs regeneration for thicker stock");
assert(keeper_z+2<=bezel_t);
assert(W<=210 && H<=210,"Bed margin exhausted");
assert(pcb_hole_length>=30 && pcb_hole_length<=64,"PCB holes exceed rail adjustment");
assert(pcb_hole_width>=0 && pcb_hole_width<=32,"PCB holes exceed carrier adjustment");
assert(pcb_length<=64 && pcb_width<=36,"PCB exceeds reserved wiring envelope");
assert(pcb_standoff>=20 && pcb_standoff<=24,"Recheck PCB/panel connector clearance");
label_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ /";
label_rows = [[2, 5, 7, 5, 5], [6, 5, 6, 5, 6], [7, 4, 4, 4, 7], [6, 5, 5, 5, 6], [7, 4, 6, 4, 7], [7, 4, 6, 4, 4], [7, 4, 5, 5, 7], [5, 5, 7, 5, 5], [7, 2, 2, 2, 7], [1, 1, 1, 5, 7], [5, 5, 6, 5, 5], [4, 4, 4, 4, 7], [5, 7, 7, 5, 5], [5, 7, 7, 7, 5], [7, 5, 5, 5, 7], [7, 5, 7, 4, 4], [7, 5, 5, 7, 1], [6, 5, 6, 5, 5], [7, 4, 7, 1, 7], [7, 2, 2, 2, 2], [5, 5, 5, 5, 7], [5, 5, 5, 5, 2], [5, 5, 7, 7, 5], [5, 5, 2, 5, 5], [5, 5, 2, 2, 2], [7, 1, 2, 4, 7], [0, 0, 0, 0, 0], [1, 1, 2, 4, 4]];
function label_index(c, i=0) = i>=len(label_chars) ? -1 : label_chars[i]==c ? i : label_index(c,i+1);
module engraved_label(value, x, y, top, scale=0.65) {
    for(i=[0:len(value)-1]) let(index=label_index(value[i]))
        if(index>=0) for(row=[0:4],col=[0:2])
            if(floor(label_rows[index][row]/pow(2,2-col))%2==1)
                translate([x+(i*4+col)*scale,y+(4-row)*scale,top-0.6]) cube([scale-0.06,scale-0.06,0.7]);
}


module rr(w,h,r=2){ hull() for(x=[-1,1],y=[-1,1]) translate([x*(w/2-r),y*(h/2-r)]) circle(r=r); }
module outline(w=W,h=H,r=R){
    if(style==5) polygon([[-w/2+7,-h/2],[w/2-7,-h/2],[w/2,-h/2+7],[w/2,h/2-7],
                          [w/2-7,h/2],[-w/2+7,h/2],[-w/2,h/2-7],[-w/2,-h/2+7]]);
    else rr(w,h,r);
}
module slab(w,h,t,r=2){linear_extrude(t) rr(w,h,r);}
module outer(t){linear_extrude(t) outline();}
module hole(x,y,z,d,h){translate([x,y,z]) cylinder(d=d,h=h);}
module slot(x,y,z,length,d,h,angle=0){translate([x,y,z]) rotate([0,0,angle]) hull(){translate([-length/2,0,0]) cylinder(d=d,h=h);translate([length/2,0,0]) cylinder(d=d,h=h);}}
module corners(z,d,h){for(x=[-1,1],y=[-1,1]) hole(x*corner_x,y*corner_y,z,d,h);}
module keeper_outline(pad=0){
    union(){
        rr(keeper_w+2*pad,keeper_h+2*pad,1+pad);
        // Local screw ears preserve material without approaching corner fasteners.
        for(x=[-1,1],y=[-1,1]) translate([x*(LW/2+3),y*(LH/2-9)]) circle(r=4+pad);
    }
}
module bezel(){
    difference(){
        outer(bezel_t);
        translate([0,0,-eps]) slab(window_w,window_h,bezel_t+2*eps,.8);
        // Rear-load sheet into REMOVABLE bezel, then fit separate keeper.
        translate([0,0,3]) slab(LW+.8,LH+.8,4,style>=3&&style<=4?.2:1.2);
        translate([0,0,keeper_z]) linear_extrude(3) keeper_outline(.25);
        corners(-eps,clearance,bezel_t+2*eps);
        for(x=[-1,1],y=[-1,1]) hole(x*(LW/2+3),y*(LH/2-9),1.0,keeper_pilot,5.6);
    }
}
module keeper(){difference(){
    linear_extrude(2) keeper_outline();
    translate([0,0,-eps]) slab(window_w,window_h,3,.8);
    for(x=[-1,1],y=[-1,1]) hole(x*(LW/2+3),y*(LH/2-9),-eps,2.8,3);
}}
module body(){difference(){
    intersection(){
        outer(D);
        union(){
            difference(){outer(D);translate([0,0,-eps]) linear_extrude(D+2*eps) outline(W-2*wall,H-2*wall,max(1,R-wall));}
            // Full-height bosses avoid the previous unsupported elevated bosses.
            for(x=[-1,1],y=[-1,1]) hole(x*corner_x,y*corner_y,0,10,D);
            // Bridges carry panel by threaded REAR inserts; front stays untouched.
            for(x=[-1,1],y=[-1,1]) hull(){
                hole(x*bridge_x,y*32.5,0,9,panel_back-bezel_t);
                hole(x*(W/2-1),y*32.5,0,7,panel_back-bezel_t);
            }
        }
    }
    corners(-eps,pilot,12); corners(D-12,pilot,13);
    // Clearance for keeper screw heads up toØ5.5 x2.5mm, incl. assembly tolerance.
    for(x=[-1,1],y=[-1,1]) hole(x*(LW/2+3),y*(LH/2-9),-eps,6.2,2.8);
    for(x=[-1,1],y=[-1,1]) hole(x*bridge_x,y*32.5,panel_back-bezel_t-9,pilot,11);
    // Slots 3mm wide: short bridges at their roofs; no support inside cable bays.
    for(x=[-66:6:66],s=[-1,1])
        translate([x,s*H/2,D*.65]) cube([3,9,min(24,D*.28)],center=true);
    // Console: shallow external vertical fluting, wall never below2.2mm.
    if(style==5) for(z=[8:7:D-6],s=[-1,1])
        translate([s*(W/2-.35),0,z]) cube([.9,H-24,2],center=true);
}}
module bridge(){difference(){
    slab(2*bridge_x+8,12,3,2);
    for(x=[-1,1]){
        hole(x*bridge_x,0,-eps,clearance,4);
        slot(x*62.5,0,-eps,3,clearance,4);
    }
}}
mounts=[for(y=[-1,1]) [carrier_xy[0],carrier_xy[1]+y*27], for(y=[-1,1]) [cassette_xy[0],cassette_xy[1]+y*31]];
function vent_safe(x)=min([for(p=mounts) (abs(x-p[0])<6.5 && abs(H/2-17-p[1])<12) ? 0:1])==1;
module lid(){difference(){
    outer(lid_t);
    corners(-eps,clearance,4);
    translate([0,ports_y,-eps]) slab(120,20,4,2);
    for(x=[-1,1]) hole(x*65,ports_y,-eps,clearance,4);
    // Carrier and cassette both removable with accessible through-bolts.
    for(y=[-1,1]) hole(carrier_xy[0],carrier_xy[1]+y*27,-eps,clearance,4);
    for(y=[-1,1]) hole(cassette_xy[0],cassette_xy[1]+y*31,-eps,clearance,4);
    for(x=[-65:6:65]) if(vent_safe(x)) translate([x,H/2-17,-eps]) slab(3,13,4,1.4);
    for(x=[-60,-30],y=[H/2-33,H/2-26]) translate([x,y,-eps]) slab(12,2.6,4,.8);
    engraved_label("INGE / AFTER HOURS",-28,H/2-7,lid_t,.65);
}}
module ports(upgrade=false){difference(){
    slab(142,32,3,3);
    for(x=[-1,1]) hole(x*65,0,-eps,clearance,4);
    // Existing power lead and USB cable pass independently. Optional bulkheads later.
    translate([-34,0,-eps]) slab(22,14,4,2);
    if(upgrade) hole(34,0,-eps,12.4,4);
    else translate([34,0,-eps]) slab(32,18,4,2);
    // Two strap slots each side of each opening for internal cable restraint.
    for(x=[-48,-20,14,54]) translate([x,0,-eps]) slab(3,12,4,.8);
}}
module carrier(){difference(){
    union(){
        slab(90,60,3,3);
        for(x=[-1,1]) translate([x*39,0,3-eps]) slab(12,46,6+eps,2);
    }
    for(x=[-1,1]) {
        slot(x*39,0,-eps,32,clearance,11,90);
        // M3 nuts slide in bottom channels, closed by the lid after adjustment.
        translate([x*39,0,-eps]) slab(6.1,42,2.5+eps,1);
    }
    for(y=[-1,1]) hole(0,y*27,-eps,clearance,4);
    // Open centre: underside wires have free air rather than a solid heat trap.
    translate([0,0,-eps]) slab(64,40,4,2);
}}
module rail(){difference(){
    slab(88,10,3,2);
    for(x=[-1,1]){
        hole(x*39,0,-eps,clearance,4);
        slot(x*23.5,0,-eps,17,3.2,4);
    }
}}
module spacer(){difference(){cylinder(d=7,h=pcb_standoff);translate([0,0,-eps]) cylinder(d=3.2,h=pcb_standoff+1);}}
module cassette(){difference(){
    union(){
        slab(78,72,2.4,5);
        // Broad oval guide: R25 minimum guide-surface bend, open-sided for ribbon.
        // Smooth oval avoids winding cables around sharp posts.
        translate([0,0,2.4-eps]) difference(){
            slab(62,52,CH-2.4+eps,25);
            translate([0,0,-eps]) slab(58,48,CH+1,23);
        }
        // Soft straps through tray slots retain coils, not pressure from the lid.
    }
    for(x=[-1,1],y=[-1,1]) translate([x*34,y*20,-eps]) slab(3,12,4,.8);
    for(y=[-1,1]) hole(0,y*31,-eps,clearance,4);
    for(x=[-24,24]) translate([x,27,-eps]) slab(12,10,4,1);
    // Ventilation and access to both tray mounting nuts.
    translate([0,0,-eps]) slab(42,22,4,8);
}}
module feet(){
    // One captive cradle per side, printed on its side as a constant section.
    // Not loose wedges: lips capture front and rear, attach with removable tape.
    length=D+bezel_t+lid_t+2;
    rotate([90,0,0]) linear_extrude(14)
        polygon([[0,0],[length+8,0],[length+8,8],[length+4,8],
                 [length+4,4],[4,4],[4,12],[0,12]]);
}
module feet_print(){translate([0,14,0]) feet();}
module coupon(){difference(){
    slab(95,60,3,3);
    for(i=[0:8]) hole(-36+i*9,-20,-eps,[1.6,1.8,2.0,2.2,2.4,2.6,2.8,3.0,3.4][i],4);
    // Top-open acrylic edge gauges with a 1.2mm floor (the old slots had a roof).
    for(i=[0:2]) translate([-39+i*28,-5-(1.0+i*.2)/2,1.2]) cube([22,1.0+i*.2,2]);
    for(i=[0:2]) hole(-28+i*28,15,-eps,12.2+i*.2,4);
}}
module acrylic_template(){rr(LW,LH,style>=3&&style<=4?.2:1);}
module assembly(explode=false){
    color("#2e3d48") translate([0,0,bezel_t]) body();
    color("#425663") translate([0,0,explode?-30:0]) bezel();
    color("#82a79b") translate([0,0,keeper_z-(explode?12:0)]) keeper();
    color([.45,.65,.7,.3]) translate([0,0,3-(explode?20:0)]) linear_extrude(acrylic_thickness) acrylic_template();
    for(y=[-1,1]) color("#b8c8c2") translate([0,y*32.5,panel_back+(explode?12:0)]) bridge();
    color("#2e3d48") translate([0,0,D+bezel_t+(explode?70:0)]) lid();
    color("#82a79b") translate([0,ports_y,D+bezel_t+3+(explode?90:0)]) ports();
    color("#839eae") translate([carrier_xy[0],carrier_xy[1],D+bezel_t-(explode?-45:0)]) rotate([180,0,0]) carrier();
    for(y=[-1,1]) color("#b8c8c2") translate([carrier_xy[0],carrier_xy[1]+y*pcb_hole_width/2,D+bezel_t-9+(explode?40:0)]) rotate([180,0,0]) rail();
    color("#bd977b") translate([cassette_xy[0],cassette_xy[1],D+bezel_t+(explode?45:0)]) rotate([180,0,0]) cassette();
    for(x=[-1,1],y=[-1,1]) color("#aabbb0") translate([carrier_xy[0]+x*pcb_hole_length/2,carrier_xy[1]+y*pcb_hole_width/2,D+bezel_t-12+(explode?40:0)]) rotate([180,0,0]) spacer();
    for(x=[-1,1]) color("#2e3d48")
        translate([x*(W/2-22)-7,-H/2-4,-5])
        multmatrix([[0,1,0,0],[0,0,1,0],[1,0,0,0],[0,0,0,1]]) feet_print();
    color("#5e9b88") translate([carrier_xy[0]-pcb_length/2,carrier_xy[1]-pcb_width/2,D+bezel_t-12-pcb_standoff-pcb_thickness+(explode?40:0)])
        cube([pcb_length,pcb_width,pcb_thickness]);
    if(!explode) color("#101418") translate([0,0,panel_front]) slab(160,80,panel_depth,.4);
}
if(part=="body") body();
else if(part=="bezel") bezel();
else if(part=="keeper") keeper();
else if(part=="lid") lid();
else if(part=="bridge") bridge();
else if(part=="carrier") carrier();
else if(part=="rail") rail();
else if(part=="spacer") spacer();
else if(part=="ports") ports();
else if(part=="ports_usb_c") ports(true);
else if(part=="cassette") cassette();
else if(part=="feet") feet_print();
else if(part=="coupon") coupon();
else if(part=="acrylic_template") acrylic_template();
else if(part=="assembly") assembly();
else if(part=="exploded") assembly(true);
else assert(false,"Unknown part");
