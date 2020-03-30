#!/usr/bin/env python
import board
import neopixel
import time
import math
import sys

pixel_pin = board.D18
COLS = 31
ROWS = 4
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
for frame in range(0,frames):
	print("{0}/{1}".format(frame,frames))
	animation.append([]);
	for y in range(0,ROWS):
		animation[frame].append([])
		for x in range(0,COLS):
			index = 3*(x+y*COLS)+frame*(COLS*ROWS*3)
			#print("{0},{1} ({3}) {2}".format(x,y,index,frame))
			animation[frame][y].append((
				int(math.pow(int(data[index+0])/255.0,GAMMA)*255),
				int(math.pow(int(data[index+1])/255.0,GAMMA)*255),
				int(math.pow(int(data[index+2])/255.0,GAMMA)*255)
			))
		if((y%2)==0):
			animation[frame][y].reverse()
f.close()
#print(animation)
frame=0;
while(True):
	for i in range(0,num_pixels):
		x = i%COLS;
		y = math.floor(i/COLS)
		pixels[i]=animation[frame][y][x]
	pixels.show();
	#print(frame)
	frame=(frame+1)%frames
	time.sleep(0.02)

