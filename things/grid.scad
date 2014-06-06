pixel_width = 16.6;
pixel_wall = 1;
pixel_height = 20;
strip_width = 11;
strip_height = 1;

pixels_x = 6;
pixels_y = 5;

remove_wall_x = 1;
remove_wall_y = 0;

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
		translate([0, (pixel_width + pixel_wall - strip_width)/2 + y * pixel_width, 0])
			cube([pixels_x * pixel_width + pixel_wall, strip_width, strip_height]);
	}
	if (remove_wall_x)
		cube([pixel_wall, pixels_y * pixel_width + pixel_wall, pixel_height]);
	if (remove_wall_y)
		cube([pixels_x * pixel_width + pixel_wall, pixel_wall, pixel_height]);
}