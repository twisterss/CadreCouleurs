// Holder to prevent the grid from moving inside the frame
// For the laser-cut grid, use holder=0

// Actual inside width of the frame
frame_width = 217;

// Size of the grid
pixel_width = 16.6;
pixel_wall = 1;
pixel_height = 20;

pixels_x = 12;
pixels_y = 10;

// Size of the protruding parts holding the grid
holder = 5;

difference() {
	cube([(frame_width - pixel_width * pixels_x - pixel_wall) / 2 + holder, (frame_width - pixel_width * pixels_y - pixel_wall) / 2 + holder, pixel_height-5]);
	translate([(frame_width - pixel_width * pixels_x - pixel_wall) / 2, (frame_width - pixel_width * pixels_y - pixel_wall) / 2, 0])
		cube([holder+1, holder+1, pixel_height-5]);
}