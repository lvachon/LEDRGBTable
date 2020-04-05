#!/usr/bin/env python3
import board
import neopixel
import time
import math
pixel_pin = board.D18
COLS = 31
ROWS = 16
num_pixels = COLS*ROWS

ORDER = neopixel.GRB

pixels = neopixel.NeoPixel(pixel_pin, num_pixels, brightness=0.25, auto_write=False, pixel_order=ORDER)

def hue2rgb(pos):
    # Input a value 0 to 255 to get a color value.
    # The colours are a transition r - g - b - back to r.
    if pos < 0 or pos > 255:
        r = g = b = 0
    elif pos < 85:
        r = int(pos * 3)
        g = int(255 - pos*3)
        b = 0
    elif pos < 170:
        pos -= 85
        r = int(255 - pos*3)
        g = 0
        b = int(pos*3)
    else:
        pos -= 170
        r = 0
        g = int(pos*3)
        b = int(255 - pos*3)
    return (r, g, b) if ORDER == neopixel.RGB or ORDER == neopixel.GRB else (r, g, b, 0)

def xy2i(x,y):
	if((y%2)==0):
		i=y*COLS+x
	else:
		i=y*COLS+(COLS-1-x)
	#print("{0},{1} = {2}".format(x,y,i))
	return i

def circle(cx,cy,r,color):
	for a in range(0,8*r):
		o = a/(4*r)*math.pi
		x=math.floor(cx+r*math.cos(o))
		y=math.floor(cy+r*math.sin(o))
		if(x<0 or y<0 or x>=COLS or y>=ROWS):
			continue
		i = xy2i(x,y)
		pixels[i]=color

def rect(x,y,w,h,color):
	for a in range(max(0,y),min(ROWS,y+h)):
		if(x>=0 and x<COLS):
			i = xy2i(x,a)
			pixels[i]=color
		if(x+w>=0 and x+w<COLS):
			i = xy2i(x+w,a)
			pixels[i]=color
	for a in range(max(0,x),min(COLS,x+w)):
		if(y>=0 and y<ROWS):
			i = xy2i(a,y)
			pixels[i]=color
		if(y+h>=0 and y+h<ROWS):
			i = xy2i(a,y+h)
			pixels[i]=color
	if(x+w>=0 and x+w<COLS and y+h>=0 and y+h<ROWS):
		pixels[xy2i(x+w,y+h)]=color

def fillRainbowSquares(cx,cy,startHue,hueStep):
	for r in range(1,max(COLS,ROWS)):
		hue = (startHue+hueStep*r) % 255
		color = hue2rgb(hue)
		rect(cx-r,cy-r,r*2,r*2,color)

def fillRainbowCircles(cx,cy,startHue,hueStep):
	for r in range(0,max(COLS,ROWS)):
		hue = (startHue + hueStep*r) % 255
		color = hue2rgb(hue)
		circle(cx,cy,r,color)
HUE = 0
CX=math.floor(COLS/2);
CY=math.floor(ROWS/2);
VX=1;
VY=1;
while True:
	HUE=HUE+1
	fillRainbowSquares(CX,CY,HUE,8);
	CX=CX+VX
	CY=CY+VY
	if CX<0:
		CX=0
		VX=1
	if CX>=COLS:
		CX=COLS-1
		VX=-1
	if CY<0:
		CY=0
		VY=1
	if CY>=ROWS:
		CY=ROWS-1
		VY=-1
	
	pixels.show();
#	time.sleep(0.001);

