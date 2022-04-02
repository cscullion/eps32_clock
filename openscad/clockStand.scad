uprightW = 15;
uprightD = 5;
uprightL = 95;
uprightCutoutD = 2.5;
uprightCutoutL = 64;
uprightHoleH = 8;
uprightHoleR = 3.6/2;
tailW = uprightW;
tailD = uprightD;
tailL = 50;
tailHoleH = tailD+0.5;
tailHoleR = 3.6/2;
tailHoleCounterR = 5.6/2;
tailHoleCounterD = 3;

module hole() {
    cylinder(h=uprightHoleH, r=uprightHoleR, center=true);
}

module hole2() {
    union() {
        cylinder(h=tailHoleH, r=tailHoleR, center=true);
        //countersink
        translate([0, 0, -2.5]) {
            cylinder(h=tailHoleCounterD, r=tailHoleCounterR, center=true);
        }
    }
}

module upright() {
    difference() {
        cube([uprightW, uprightD, uprightL]);
        //upper hole
        translate([uprightW/2,uprightD/2,uprightL-9]) {
            rotate([90, 0, 0]) {
                hole();
            }
        }
        //lower hole
        translate([uprightW/2, uprightD/2,uprightL-9-74]) {
            rotate([90, 0, 0]) {
                hole();
            }
        }
        //cutout
        translate([-0.5, -0.5, (uprightL-uprightCutoutL)/2]) {
            cube([uprightW+1, uprightCutoutD, uprightCutoutL]);
        }
    }
}

module tail() {
    difference() {
        cube([tailW, tailL, tailD]);
        translate([(tailW-8)/2, tailL-7, tailD/2]) {
            hole2();
        }
        translate([((tailW-8.5)/2+8.5), tailL-7, tailD/2]) {
            hole2();
        }
    }
}

module leg() {
    union() {
        translate([0,0,4.5]){
            rotate(a=-20, v=[tailD,0,0]) {
                upright();
            }
        }
        tail();
    }
}

leg();


