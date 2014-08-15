// Grid to separate pixels on the frame.
// Can be 3D-printed in 1 or 4 parts.
// See different combinations at the end of the file

// Basic dimensions
pixel_width = 16.6;
pixel_wall = 1;
pixel_height = 20;
strip_width = 11;
strip_height = 0.8;
strip_big_height = 2.8;

// Grid drawing module
// pixels_x, pixels_y: Grid size
// remove_wall_x, remove_wall_x: Leave space to put another grid side by side
// bigger_strip_start, bigger_strip_end: Leave space for soldering on the end
module grid(pixels_x, pixels_y, remove_wall_x, remove_wall_y, bigger_strip_start, bigger_strip_end) {
	difference() {
		union() {
			for (x = [0 : pixels_x]) {
				translate([x * pixel_width, 0, 0])
					cube([pixel_wall, pixels_y * pixel_width + pixel_wall, pixel_height]);
			}
			for (y = [0 : pixels_y]) {
				translate([0, y * pixel_width, 0])
					cube([pixels_x * pixel_width + pixel_wall, pixel_wall, pixel_height]);
			}
		}
		for (y = [0 : pixels_y-1]) {
			translate([0, (pixel_width + pixel_wall - strip_width)/2 + y * pixel_width, 0]) {
				cube([pixels_x * pixel_width + pixel_wall, strip_width, strip_height]);
				if (bigger_strip_start)
					cube([pixel_wall, strip_width, strip_big_height]);
				if (bigger_strip_end)
					translate([pixels_x * pixel_width, 0, 0])
						cube([pixel_wall, strip_width, strip_big_height]);
			}
		}
		if (remove_wall_x)
			cube([pixel_wall, pixels_y * pixel_width + pixel_wall, pixel_height]);
		if (remove_wall_y)
			cube([pixels_x * pixel_width + pixel_wall, pixel_wall, pixel_height]);
	}
}

// Full grid made of one part
grid(12, 10, 0, 0, 1, 1);

// Grid made of 4 parts for smaller printers (print both twice)
//grid(6, 5, 1, 0, 0, 0);
//translate([6*pixel_width+5, 0, 0])
//	grid(6, 5, 0, 1, 0, 0);