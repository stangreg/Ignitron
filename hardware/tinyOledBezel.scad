// Bezel for tiny Oled 0.96"
// Stiftleiste ist oben

$fn=20;
CLEARANCE=0.3;

WALL_THICKNESS=2.5;

OLED_XLENGTH=27.6 + CLEARANCE;
OLED_YLENGTH=27.8 + CLEARANCE;
OLED_XLEN_VISIBLE=25;
OLED_YLEN_VISIBLE=14.5;
OLED_FromTopToVisible=5.0+WALL_THICKNESS;
OLED_R=1;  // holes on the board

WALL_FRONT=1;
OUTER_LENGTH=WALL_THICKNESS*2 + 28;  // 28 is OLED_XLENGTH rounded up
EDGE_SIZE=4;
EDGE_ZLEN=1.9;

// Select "pin" or "screw"
mode = "pin";
//mode = "screw";

// Set variables for Pin or for screwholes
// Pin:
EDGE_PinHeight=4.2;
EDGE_PinRadius_bottom=0.9;
EDGE_PinRadius_top=0.8;
// Screwhole: 0.5
EDGE_ScrewRadius=0.5;

CONNECTOR_LENGTH=OLED_XLENGTH;
CONNECTOR_WIDTH = 2.5;
CONNECTOR_HEIGHT = 4;
CONNECTOR_HOLE_RADIUS = 0.85;
CONNECTOR_HOLE_DEPTH = 5;

STRUTS_THICKNESS=0.75;
STRUTS_HEIGHT=10;

BOX_ZLENGTH=8;
R_SCREW=1.2;

// Podest für Displayauflage
// Der zylinder ist entweder herausstehend (ohne differene), 
// oder kann als Bohrloch konfiguriert werden (mit difference)
module platform(x,y,dx,dy) {
    translate([x,y,WALL_FRONT+EDGE_ZLEN/2]) {    
        //Option: Pin
        if(mode == "pin"){
            translate([dx,dy,0]) cube([EDGE_SIZE, EDGE_SIZE, EDGE_ZLEN], center=true);
            translate([0,0,EDGE_ZLEN/2]) cylinder(r1=EDGE_PinRadius_bottom,r2=EDGE_PinRadius_top,h=EDGE_PinHeight);
        }
        else if(mode == "screw"){
            // Option: Screwholes
            difference(){ 
                translate([dx,dy,0]) cube([EDGE_SIZE, EDGE_SIZE, EDGE_ZLEN], center=true);    
                cylinder(r=EDGE_ScrewRadius,h=EDGE_ZLEN);
            }
        }
        else {
            translate([dx,dy,0]) cube([EDGE_SIZE, EDGE_SIZE, EDGE_ZLEN], center=true);
        }
    }
}

module connector(x,y) {
    translate([x, y, WALL_FRONT+BOX_ZLENGTH+STRUTS_HEIGHT+1]) rotate([0,180,0]){
        difference(){
            cube([CONNECTOR_WIDTH,CONNECTOR_LENGTH,CONNECTOR_HEIGHT],center=true);
            connectorHoles();
        }
        struts();
    }
}

module connectorHoles() {
    translate([0,CONNECTOR_LENGTH/2-2,-CONNECTOR_HEIGHT/2-0.1]) cylinder(r=CONNECTOR_HOLE_RADIUS, h=CONNECTOR_HOLE_DEPTH);
    translate([0,-CONNECTOR_LENGTH/2+2,-CONNECTOR_HEIGHT/2-0.1]) cylinder(r=CONNECTOR_HOLE_RADIUS,h=CONNECTOR_HOLE_DEPTH);
    
}

// Schraublöcher außen
module screwHoles() {
translate([-1-OUTER_LENGTH/2,OLED_YLENGTH/2-5,BOX_ZLENGTH-3]) rotate([0,70,0])
    cylinder(r=R_SCREW,h=10);
translate([-1-OUTER_LENGTH/2,-OLED_YLENGTH/2+5,BOX_ZLENGTH-3]) rotate([0,70,0])
    cylinder(r=R_SCREW,h=10);
translate([1+OUTER_LENGTH/2,-OLED_YLENGTH/2+5,BOX_ZLENGTH-3]) rotate([0,-70,0])
    cylinder(r=R_SCREW,h=10);
translate([1+OUTER_LENGTH/2,OLED_YLENGTH/2-5,BOX_ZLENGTH-3]) rotate([0,-70,0])
    cylinder(r=R_SCREW,h=10);
}

module struts() {
    translate([0,CONNECTOR_LENGTH/5,CONNECTOR_HEIGHT/2]) 
        cylinder(r=STRUTS_THICKNESS, h=STRUTS_HEIGHT);
    translate([0,-CONNECTOR_LENGTH/5,CONNECTOR_HEIGHT/2])
        cylinder(r=STRUTS_THICKNESS, h=STRUTS_HEIGHT);
}

// Vier Wände mit Schraublöchern
difference() {
    translate([0,0,BOX_ZLENGTH/2])
        cube([OUTER_LENGTH,OUTER_LENGTH,BOX_ZLENGTH], center=true);

    translate([0,OUTER_LENGTH/2 - OLED_YLEN_VISIBLE/2 - OLED_FromTopToVisible, 0])
        cube([OLED_XLEN_VISIBLE,OLED_YLEN_VISIBLE,100], center=true);
    translate([0,0,BOX_ZLENGTH/2 + WALL_FRONT])
    cube([OLED_XLENGTH+1, OLED_YLENGTH+1,BOX_ZLENGTH], center=true);
    screwHoles();
}

// Vier Eckpodeste, auf denen das Display aufliegt
// (müssen vielleicht größer gemacht werden, wenn wir schrauben wollen)
platform(-OLED_XLENGTH/2+EDGE_SIZE/2, -OLED_YLENGTH/2+EDGE_SIZE/2,-1,-1);
platform(OLED_XLENGTH/2-EDGE_SIZE/2, OLED_YLENGTH/2-EDGE_SIZE/2,1,1);
platform(OLED_XLENGTH/2-EDGE_SIZE/2, -OLED_YLENGTH/2+EDGE_SIZE/2,1,-1);
platform(-OLED_XLENGTH/2+EDGE_SIZE/2, OLED_YLENGTH/2-EDGE_SIZE/2,-1,1);

// Verbindungsteile zum Fixieren des Displays
//connector(-OLED_XLENGTH/2+EDGE_SIZE/2, 0);
//connector(OLED_XLENGTH/2-EDGE_SIZE/2, 0);
connector(-OLED_XLENGTH/2-WALL_THICKNESS/2, 0);
connector(OLED_XLENGTH/2+WALL_THICKNESS/2, 0);

//Überstehender Rand
difference() {
    translate([0,0,WALL_FRONT/2])
      minkowski() {
        cube([OUTER_LENGTH+1, OUTER_LENGTH+1, WALL_FRONT], center=true);
        cylinder(r=2, h=0.001);
      }

    cube([OUTER_LENGTH,OUTER_LENGTH,BOX_ZLENGTH], center=true);
}

