#!/usr/bin/env python3

import sys
import math


COLS = 31
ROWS = 16
num_pixels = COLS*ROWS
#GRB ORDER

f = open(sys.argv[1],"r")
format = f.readline()
comment = f.readline()
dimensions = f.readline().split()
maxval = f.readline()
data = f.read()
data = data.split()
frames = (int)(len(data)/(num_pixels*3));
print(len(data))
print(frames)
GAMMA = 2
animation = bytearray(num_pixels*3*frames)
cnt=0
for frame in range(0,frames):
	print("{0}/{1}".format(frame,frames))

	for y in range(0,ROWS):
		for x in range(0,COLS):
			if((y%2)==0):
				px=COLS-1-x
			else:
				px=x
			index = 3*(px+y*COLS)+frame*(num_pixels*3)
			animation[cnt]=int(math.pow(int(data[index+1])/255.0,GAMMA)*255)  #G
			animation[cnt+1]=int(math.pow(int(data[index+0])/255.0,GAMMA)*255)#R
			animation[cnt+2]=int(math.pow(int(data[index+2])/255.0,GAMMA)*255)#B
			cnt=cnt+3

f.close()

with open(sys.argv[2],"wb") as file:
	file.write(animation)
