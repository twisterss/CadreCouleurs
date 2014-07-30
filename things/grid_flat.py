#!/bin/python

# Draw the shape of the grid for laser cutting as grid_flat.svg
# Use the svgwrite module

import sys
import svgwrite

pixelWidth = 16.6
pixelWall = 1.1
pixelHeight = 20.
stripWidth = 11.
stripHeight = 0.6
stripBigHeight = 2.5
wallEnd = 3.
margin = 0.05
frameHeight = 2

pixelsX = 10
pixelsY = 12
spareParts = 0

imageWidth = 384
imageHeight = 790
flipOrientation = True

lineWidth = 0.01

def svgPoint(point):
	"""
	Returns a point formatted for SVG
	"""
	if flipOrientation:
		return (str(point[1])+"mm", str(point[0])+"mm")
	else:
		return (str(point[0])+"mm", str(point[1])+"mm")

def drawLine(dwg, currentPoint, x=0, y=0, endPoint = None, draw=True):
	"""
	Adds a line to the drawing, update the current point
	"""
	if endPoint is None:
		endPoint = (currentPoint[0] + x, currentPoint[1] + y)
	if draw:
		dwg.add(dwg.line(svgPoint(currentPoint), svgPoint(endPoint), stroke="rgb(0, 0, 255)", stroke_width = str(lineWidth) + "mm", fill = "none"))
	currentPoint[0] = endPoint[0]
	currentPoint[1] = endPoint[1]

def drawWall(dwg, currentPoint, pixels, stripSpace, finish = True):
	"""
	Draw a wall, update only y on currentPoint
	"""
	startPoint = list(currentPoint)
	# Draw the first wall end
	if stripSpace > 0 and frameHeight > 0:
		# Draw the cut to leave space for the surrounding frame
		tmpPoint = list(currentPoint)
		drawLine(dwg, tmpPoint, x = wallEnd+margin, draw = False)
		drawLine(dwg, tmpPoint, y = frameHeight)
		drawLine(dwg, tmpPoint, x = -wallEnd-margin)
	drawLine(dwg, currentPoint, y = pixelHeight)
	drawLine(dwg, currentPoint, x = wallEnd, draw = finish)
	if stripSpace == 0 and frameHeight > 0:
		# Draw the cut to leave space for the surrounding frame
		tmpPoint = list(currentPoint)
		drawLine(dwg, tmpPoint, y = -frameHeight, draw = False)
		drawLine(dwg, tmpPoint, x = -wallEnd)
	# Draw each pixel
	for i in range(pixels):
		drawLine(dwg, currentPoint, y = -pixelHeight/2 - margin)
		drawLine(dwg, currentPoint, x = pixelWall + margin)
		drawLine(dwg, currentPoint, y = pixelHeight/2 + margin)
		if stripSpace > 0:
			drawLine(dwg, currentPoint, x = (pixelWidth - pixelWall - margin - stripWidth) / 2, draw = finish)
			drawLine(dwg, currentPoint, y = -stripSpace)
			drawLine(dwg, currentPoint, x = stripWidth)
			drawLine(dwg, currentPoint, y = stripSpace)
			drawLine(dwg, currentPoint, x = (pixelWidth - pixelWall - margin - stripWidth) / 2, draw = finish)
		else:
			drawLine(dwg, currentPoint, x = pixelWidth - pixelWall - margin, draw = finish)
	# Draw the last wall end
	drawLine(dwg, currentPoint, y = -pixelHeight/2 - margin)
	drawLine(dwg, currentPoint, x = pixelWall + margin)
	drawLine(dwg, currentPoint, y = pixelHeight/2 + margin)
	drawLine(dwg, currentPoint, x = wallEnd, draw = finish)
	if stripSpace == 0 and frameHeight > 0:
		# Draw the cut to leave space for the surrounding frame
		tmpPoint = list(currentPoint)
		drawLine(dwg, tmpPoint, y = -frameHeight, draw = False)
		drawLine(dwg, tmpPoint, x = -wallEnd)
	drawLine(dwg, currentPoint, y = -pixelHeight)
	if stripSpace > 0 and frameHeight > 0:
		# Draw the cut to leave space for the surrounding frame
		tmpPoint = list(currentPoint)
		drawLine(dwg, tmpPoint, x = -wallEnd-margin, draw = False)
		drawLine(dwg, tmpPoint, y = frameHeight)
		drawLine(dwg, tmpPoint, x = wallEnd+margin)
	# Finish with the big cut part
	drawLine(dwg, currentPoint, endPoint = startPoint)
	currentPoint[1]+= pixelHeight

def main(args):
	"""
	Generate the SVG file
	"""
	dwg = svgwrite.Drawing('grid_flat.svg', profile='tiny', size=svgPoint((imageWidth, imageHeight)))
	currentPoint = [10, 10]
	for i in range(pixelsY-1+spareParts+1):
		drawWall(dwg, currentPoint, pixelsX, stripHeight, False)
	for i in range(2+spareParts):
		drawWall(dwg, currentPoint, pixelsX, stripBigHeight, False)
	for i in range(pixelsX+spareParts):
		drawWall(dwg, currentPoint, pixelsY, 0, False)
	drawWall(dwg, currentPoint, pixelsY, 0, True)
	dwg.save()
	exit(0)


if __name__ == '__main__':
	main(sys.argv)