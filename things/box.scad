$fn = 50;

content_width = 53.5;
content_depth = 72;
content_height = 46;
wall = 1.5;
screw_sep = 3;
screw_bottom = 27;
screw_diam = 3.5;
screw_width = 6.3;
screw_height = 7;
screw_depth = 4;
screw_div = 1;

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
					translate([10, 0, content_height - 3])
						cube([6, wall, 3 + wall]);
					translate([13, wall, content_height - 3])
						rotate([90, 0, 0])
							cylinder(h = wall, r = 3);
					// USB hole
					translate([31.5, 0, 5])
						cube([13, wall, 11]);
					// Ethernet hole
					translate([29.75, 0, 22.75])
						cube([16.5, wall, 13.5]);
					// output hole
					translate([23, 0, content_height - 3])
						cube([6, wall, 3 + wall]);
					translate([26, wall, content_height - 3])
						rotate([90, 0, 0])
							cylinder(h = wall, r = 3);
				}
			}
			// Holding part 1
			translate([wall, -(screw_sep + screw_width + wall*2), screw_bottom + wall]) {
				difference() {
					cube([wall*2 + screw_depth, screw_sep + screw_width + wall*2, screw_height + wall]);
					translate([wall, wall, wall])
						cube([screw_depth, screw_width, screw_height]);
					translate([0, screw_width/2 + wall, screw_height/2 + wall])
						rotate([0, 90, 0])
							cylinder(h = wall*2 + screw_depth, r = screw_diam/2);
				}
			}
			// Holding part 2
			translate([content_width - screw_depth - wall, -(screw_sep + screw_width + wall*2), screw_bottom + wall]) {
				difference() {
					cube([wall*2 + screw_depth, screw_sep + screw_width + wall*2, screw_height + wall]);
					translate([wall, wall, wall])
						cube([screw_depth, screw_width, screw_height]);
					translate([0, screw_width/2 + wall, screw_height/2 + wall])
						rotate([0, 90, 0])
							cylinder(h = wall*2 + screw_depth, r = screw_diam/2);
				}
			}
		}
	}
}

box();
end();