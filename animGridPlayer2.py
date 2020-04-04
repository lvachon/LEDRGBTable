#!/usr/bin/env python
import board
import neopixel
import time
import math
import sys

pixel_pin = board.D18
COLS = 31
ROWS = 15
num_pixels = COLS*ROWS

ORDER = neopixel.GRB

pixels = neopixel.NeoPixel(pixel_pin, num_pixels, brightness=0.25, auto_write=False, pixel_order=ORDER)

def xy2i(x,y):
        if((y%2)==0):
                i=y*COLS+x
        else:
                i=y*COLS+(COLS-1-x)
        return i

animation = []
f = open(sys.argv[1],"r")
format = f.readline()
comment = f.readline()
dimensions = f.readline().split()
maxval = f.readline()
data = f.read()
data = data.split()
frames = (int)(len(data)/(3*COLS*ROWS));
print(len(data))
print(frames)
GAMMA = 2
animation = []
for frame in range(0,frames):
	print("{0}/{1}".format(frame,frames))
	pixels[(int)(num_pixels*frame/frames)]=(64,0,0);
	pixels.show();
	for y in range(0,ROWS):
		for x in range(0,COLS):
			if((y%2)==1):
				index = 3*((COLS-1-x)+y*COLS)+frame*(num_pixels*3)
			else:
				index = 3*(x+y*COLS)+frame*(num_pixels*3)
			animation.append((
				int(math.pow(int(data[index+0])/255.0,GAMMA)*255),
				int(math.pow(int(data[index+1])/255.0,GAMMA)*255),
				int(math.pow(int(data[index+2])/255.0,GAMMA)*255)
			))
f.close()
#print(animation)
frame=0;

while(1):
	#print(frame)
	ot = time.perf_counter()
	for i in range(frame*num_pixels,frame*num_pixels+num_pixels):
		pixels[i-frame*num_pixels]=animation[i]
	pixels.show();
	frame=(frame+1)%frames
	#print(time.perf_counter()-ot)
	time.sleep(max(0,0.03-(time.perf_counter()-ot)))

