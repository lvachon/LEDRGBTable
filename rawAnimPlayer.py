#!/usr/bin/env python3
import board
import neopixel_write
import digitalio
import time
import math
import sys

pin = digitalio.DigitalInOut(board.D18)
pin.direction = digitalio.Direction.OUTPUT
#pixel_pin = board.D18

COLS = 31
ROWS = 15
num_pixels = COLS*ROWS
#GRB ORDER


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
animation = bytearray(num_pixels*3*frames)
cnt=0
for frame in range(0,frames):
	print("{0}/{1}".format(frame,frames))
	#pixels[(int)(num_pixels*frame/frames)]=(64,0,0);
	#pixels.show();
	for y in range(0,ROWS):
		for x in range(0,COLS):
			if((y%2)==1):
				index = 3*((COLS-1-x)+y*COLS)+frame*(num_pixels*3)
			else:
				index = 3*(x+y*COLS)+frame*(num_pixels*3)
			animation[cnt]=int(math.pow(int(data[index+1])/255.0,GAMMA)*255)  #G
			animation[cnt+1]=int(math.pow(int(data[index+0])/255.0,GAMMA)*255)#R
			animation[cnt+2]=int(math.pow(int(data[index+2])/255.0,GAMMA)*255)#B
			cnt=cnt+3

f.close()
#print(animation)
frame=0;
ilace=0
while(1):
	neopixel_write.neopixel_write(pin,animation[3*num_pixels*frame:3*num_pixels*(frame+1)])
	#ilace=1-ilace
	frame=(frame+1)%frames
	#print(time.perf_counter()-ot)
	time.sleep(0.01)

