/* MINIMAL / FRAME — two printed parts, mm. Prototype fit must be checked.
   Panel loads from FRONT, bolts through the four integral rear mounting ears.
   Existing USB-C and barrel power sockets face directly out of the rear.
   See README.md. Hardware envelopes are illustrative, not extra print parts. */
part = "assembly"; // [assembly,exploded,frame,rear,fit_corner,board_gauge,acrylic_template,panel_mock,pcb_mock,usb_mock,power_mock]
panel_w = 160;
panel_h = 80;
panel_depth = 14.5;
panel_clearance = 0.35; // per side; close sliding fit, never a press fit
acrylic_t = 1;
acrylic_w = 164;
acrylic_h = 84;
wall = 2.4;
outer_w = 176;
outer_h = 96;
rear_z = 108; // direct USB-C: board length + panel connector clearance
pcb_l = 58; // provisional: measure actual board
pcb_w = 29;
pcb_t = 1.6;
rear_wall = 1.6;
pcb_slot = 2.2; // board thickness + 0.6mm total clearance
pcb_stop = 2.8; // PCB end position behind outer rear face
usb_open_x = 12;
usb_open_y = 20;
power_open_d = 13;
pcb_x = 51; // PCB lies in YZ; component face points toward +X
power_l = 35; // existing barrel-to-terminal adapter, provisional envelope
power_w = 16;
power_h = 14;
$fn = 40;
eps = 0.04;
front_lip = 1.6;
lens_seat = acrylic_t + 0.3;
panel_front = lens_seat + front_lip + 2.9;
panel_back = panel_front + panel_depth;
frame_depth = panel_back + 3;
rear_t = 1.8;
leg_h = rear_z - frame_depth;
cx = outer_w/2 - 4;
cy = outer_h/2 - 4;
power_x = -43;
power_y = 0;
assert(panel_w+2*panel_clearance<acrylic_w);
assert(panel_h+2*panel_clearance<acrylic_h);
assert(pcb_l<=62 && pcb_w<=33,"Recheck board guides and USB access for larger PCB");
assert(rear_z-(pcb_stop+pcb_l+6)>panel_back+20,"Increase rear depth for PCB guides/panel connector clearance");
assert(pcb_slot>pcb_t && pcb_slot<=pcb_t+1);
module rr(w,h,r=2){hull() for(x=[-1,1],y=[-1,1]) translate([x*(w/2-r),y*(h/2-r)]) circle(r=r);}
module slab(w,h,t,r=2){linear_extrude(t) rr(w,h,r);}
module hole(x,y,z,d,h){translate([x,y,z]) cylinder(d=d,h=h);}
module bar(x1,y1,x2,y2,w,t){hull(){hole(x1,y1,0,w,t);hole(x2,y2,0,w,t);}}
module perimeter(t,width=wall){difference(){slab(outer_w,outer_h,t,4);translate([0,0,-eps]) slab(outer_w-2*width,outer_h-2*width,t+2*eps,max(.5,4-width));}}
module front_frame(){difference(){
    union(){
        perimeter(frame_depth);
        // Four through-bolt bosses, outside the panel and acrylic.
        for(x=[-1,1],y=[-1,1]) hole(x*cx,y*cy,0,7,frame_depth);
        // Front ledge with a 45-degree underside, printed rear face down.
        difference(){
            slab(outer_w,outer_h,8.2,4);
            translate([0,0,-eps]) slab(panel_w+2*panel_clearance,panel_h+2*panel_clearance,lens_seat+front_lip+eps,.6);
            hull(){
                translate([0,0,lens_seat+front_lip]) slab(panel_w+2*panel_clearance,panel_h+2*panel_clearance,.05,.6);
                translate([0,0,8.2]) slab(outer_w-2*wall,outer_h-2*wall,.05,1.6);
            }
        }
        // Integral mounting ears, short panel bolts; these touch the print bed.
        for(x=[-1,1],y=[-1,1]) translate([0,0,panel_back])
            bar(x*62.5,y*32.5,x*(outer_w/2-wall/2),y*32.5,8,3);
        // Side guides are rooted in the rear face: no suspended cantilevers.
        for(x=[-1,1],y=[-1,1]) translate([x*(panel_w/2+panel_clearance+(outer_w/2-panel_w/2-panel_clearance)/2),y*18,(8.2+frame_depth)/2])
            cube([outer_w/2-panel_w/2-panel_clearance,5,frame_depth-8.2],center=true);
        for(x=[-1,1],y=[-1,1]) translate([x*30,y*(panel_h/2+panel_clearance+(outer_h/2-panel_h/2-panel_clearance)/2),(8.2+frame_depth)/2])
            cube([5,outer_h/2-panel_h/2-panel_clearance,frame_depth-8.2],center=true);
    }
    // Continuous loading path from the front down to the panel mounting ears.
    translate([0,0,-eps]) slab(panel_w+2*panel_clearance,panel_h+2*panel_clearance,panel_back+eps,.6);
    translate([0,0,-eps]) slab(acrylic_w+.6,acrylic_h+.6,lens_seat+eps,1.3);
    for(x=[-1,1],y=[-1,1]){
        hole(x*cx,y*cy,-eps,3.4,frame_depth+2*eps);
        hole(x*62.5,y*32.5,panel_back-eps,3.4,3+2*eps);
    }
}}
// Restrict guide shapes and bosses to the outer envelope.
module frame_assembled(){intersection(){front_frame();slab(outer_w,outer_h,frame_depth,4);}}
module frame_print(){translate([0,0,frame_depth]) rotate([180,0,0]) frame_assembled();}
module board_guides(){
    // PCB slides USB-end first down two grooves. One 3mm tie retains its far end.
    rail_end=pcb_stop+pcb_l+6;
    for(y=[-1,1]) difference(){
        union(){
            translate([pcb_x-2.2,y>0?pcb_w/2-.8:-pcb_w/2-3.5,rear_t])
                cube([6,4.3,rail_end-rear_t]);
            // Continuous thin fin connects each guide to the case side.
            translate([pcb_x+3.7,y*(pcb_w/2+2.4)-.8,rear_t])
                cube([outer_w/2-pcb_x-3.7,1.6,rail_end-rear_t]);
        }
        translate([pcb_x+(pcb_t-pcb_slot)/2,y>0?pcb_w/2-.85:-pcb_w/2-.3,pcb_stop])
            cube([pcb_slot,1.15,rail_end-pcb_stop+eps]);
        // Tie goes over the PCB's far edge, clear of its components.
        translate([pcb_x-2.3,y*(pcb_w/2+1.5)-1.6,pcb_stop+pcb_l+.2])
            cube([7.5,3.2,3.4]);
    }
}
module power_cradle(){
    // A strapped cradle holds the existing adapter against the rear port shoulder.
    for(x=[-1,1]) difference(){
        translate([power_x+x*(power_w/2+2)-1.2,-power_h/2-5,rear_t])
            cube([2.4,power_h+10,power_l]);
        for(z=[11,26],y=[-1,1]) translate([power_x+x*(power_w/2+2)-1.3,y*(power_h/2+1.2)-1.7,z])
            cube([2.6,3.4,3.4]);
    }
}
module rear(){difference(){
    union(){
        slab(outer_w,outer_h,rear_t,4);
        perimeter(leg_h,rear_wall);
        for(x=[-1,1],y=[-1,1]) hole(x*cx,y*cy,0,7,leg_h);
        board_guides();
        power_cradle();
    }
    for(x=[-1,1],y=[-1,1]) hole(x*cx,y*cy,leg_h-11,2.6,12);
    // Rear-facing openings: the actual connectors, no extension or panel socket.
    translate([pcb_x+pcb_t/2+1.6,0,-eps]) slab(usb_open_x,usb_open_y,rear_t+2*eps,2);
    hole(power_x,power_y,-eps,power_open_d,rear_t+2*eps);
    // Small slot roofs and generous open area reduce material and retain airflow.
    for(x=[-68:8:68],y=[-1,1]) translate([x,y*30,-eps]) slab(3,15,rear_t+2*eps,1.4);
    for(x=[-66:12:66],y=[-1,1],z=[24,55])
        translate([x,y*(outer_h/2-.5),z]) cube([3,5,18],center=true);
}}

module panel(){difference(){
    translate([-panel_w/2,-panel_h/2,panel_front]) cube([panel_w,panel_h,panel_depth]);
    for(x=[-1,1],y=[-1,1]) hole(x*62.5,y*32.5,panel_back-4,2.5,5);
}}
module pcb(){
    translate([pcb_x,-pcb_w/2,pcb_stop]) cube([pcb_t,pcb_w,pcb_l]);
}
module usb_socket(){translate([pcb_x+pcb_t,-4.5,pcb_stop-1.5]) cube([3.2,9,7]);}
module power_adapter(){
    translate([power_x-power_w/2,-power_h/2,rear_t]) cube([power_w,power_h,power_l]);
    hole(power_x,0,.2,11,rear_t);
}

module back_transform(){translate([0,0,rear_z]) mirror([0,0,1]) children();}
module assembly(explode=0){
    color("#344749") frame_assembled();
    color([.7,.85,.87,.25]) translate([0,0,.3-18*explode]) linear_extrude(acrylic_t) rr(acrylic_w,acrylic_h,1);
    color("#111b20") panel();
    translate([0,0,45*explode]) back_transform(){
        color("#344749") rear();
        color("#42917a") pcb();
        color("#bbc5c7") usb_socket();
        color("#353d3d") power_adapter();
    }
}
if(part=="frame") frame_print();
else if(part=="rear") rear();
else if(part=="fit_corner") intersection(){frame_print();translate([-outer_w/2-1,-outer_h/2-1,-eps]) cube([34,31,frame_depth+2*eps]);}
else if(part=="board_gauge") union(){
    translate([pcb_x-3,-pcb_w/2-4,0]) cube([8,pcb_w+8,rear_t]);
    intersection(){board_guides();translate([pcb_x-3,-pcb_w/2-4,0]) cube([8,pcb_w+8,12]);}
}
else if(part=="panel_mock") panel();
else if(part=="pcb_mock") pcb();
else if(part=="usb_mock") usb_socket();
else if(part=="power_mock") power_adapter();
else if(part=="acrylic_template") rr(acrylic_w,acrylic_h,1);
else if(part=="assembly") assembly();
else if(part=="exploded") assembly(1);
else assert(false,"Unknown part");
