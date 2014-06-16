$fn = 50;

content_width = 53.5;
content_depth = 72;
content_height = 46;
wall = 1.5;
screw_sep = 5;
screw_bottom = 21;
screw_diam = 3.5;
screw_width = 6.3;
screw_height = 7.5;
screw_depth = 4;

module box() {
	difference() {
		cube([content_width + wall*2, content_depth + wall, content_height + wall * 2]);
		translate([wall, wall, wall]) {
			cube([content_width, content_depth, content_height]);
		}
	}
}

module end() {
	translate([0, content_depth + wall, 0]) {
		union() {
			difference() {
				cube([content_width + wall * 2, wall, content_height + wall * 2]);
				translate([wall, 0, wall]) {
					// Power hole
					translate([content_width-10, 0, content_height - 3])
						cube([6, wall, 3 + wall]);
					translate([content_width-7, wall, content_height - 3])
						rotate([90, 0, 0])
							cylinder(h = wall+1, r = 3);
					// USB hole
					translate([31.5, 0, 4.4])
						cube([13, wall, 11]);
					// Ethernet hole
					translate([29.75, 0, 18.6])
						cube([16.5, wall, 13.5]);
					// Output plug hole
					translate([12.5, 0, content_height + wall - 6])
						cube([4, wall, 6]);
					translate([14.7, wall, content_height - 10.7])
						rotate([90, 0, 0])
							cylinder(h = wall+1, r = 7.7);
					// Button hole
					translate([34, wall, content_height - 6])
						rotate([90, 0, 0])
							cylinder(h = wall+1, r = 3.35);
				}
			}
			// Holding part 1
			translate([wall, -(screw_sep + screw_width + wall*2), screw_bottom + wall]) {
				difference() {
					cube([wall*3 + screw_depth, screw_sep + screw_width + wall*2, screw_height + wall]);
					translate([wall*1.5, wall, wall])
						cube([screw_depth, screw_width, screw_height]);
					translate([0, screw_width/2 + wall, screw_height/2 + wall])
						rotate([0, 90, 0])
							cylinder(h = wall*3 + screw_depth, r = screw_diam/2);
				}
			}
		}
	}
}

//box();
end();