#!/bin/python

# Script to generate colors sorted by similarity.
# Can output in C or HTML format
# Usage: python colors.py C|html

import sys
import colorsys

# Color variation parameters
HUE_VALUES = 85
SAT_VALUES = 4

def generate_colors():
	"""
	Generate a list of colors
	"""
	colors = []
	for hue in range(HUE_VALUES):
		hue = float(hue) / HUE_VALUES
		for sat in range(SAT_VALUES):
			if SAT_VALUES > 1:
				sat = float(sat) / (2*(SAT_VALUES-1)) + 0.5
			else:
				sat = 1
			color = colorsys.hsv_to_rgb(hue, sat, 1)
			color = (int(color[0]*255), int(color[1]*255), int(color[2]*255))
			colors.append(color)
	return colors

def main(args):
	"""
	Generate and print the list of colors
	"""
	# Generate colors
	colors = generate_colors()
	# Check the output format
	output = "C"
	if len(args) > 1:
		output = args[1]
	else:
		print("Usage: python colors.py C|html")
		print("Script to generate colors sorted by similarity.")
		print("Can output in C or HTML format")
		exit(1)
	# Output
	if output == "html":
		# HTML format
		for color in colors:
			print('<div style="display: block; margin: 0; padding: 0; width: 200px; height: 5px; background-color: rgb(' + str(color[0]) + ',' + str(color[1]) + ',' + str(color[2]) + ');">&nbsp;</div>')
	else:
		# C format
		sys.stdout.write('{')
		first = True
		for color in colors:
			if not first:
				sys.stdout.write(", ")
			first = False
			sys.stdout.write('{' + str(color[0]) + ', ' + str(color[1]) + ', ' + str(color[2]) + '}')
		sys.stdout.write('}\n')
	exit(0)


if __name__ == '__main__':
	main(sys.argv)