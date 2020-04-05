#!/usr/bin/env python3
import board
import neopixel
import random
import time
#import math

pixel_pin = board.D18
COLS = 31
ROWS = 16
num_pixels = COLS*ROWS

ORDER = neopixel.GRB

pixels = neopixel.NeoPixel(pixel_pin, num_pixels, brightness=0.25, auto_write=False, pixel_order=ORDER)
options = (
(255,0,0),
(0,255,0),
(0,0,255),
(0,0,0)
)
random.seed()
field = [options[random.randint(0,3)] for n in range(0,num_pixels)]
nextfield = [field[n] for n in range(0,num_pixels)]
def neighbor(i,n):
	x=i%COLS
	y=(int)(i/COLS)
	if(n==0 and y>0):
		return field[i-COLS]
	if(n==1 and x<COLS-1):
		return field[i+1]
	if(n==2 and y<ROWS-1):
		return field[i+COLS]
	if(n==3 and x>0):
		return field[i-1]
	return (0,0,0)

STEPDIV=16
def step(i):
	iR=0
	iP=0
	iS=0
	influence = (0,0,0)
	for j in range(0,4):
		n = neighbor(i,j)
#		influence=influence+neighbor(i,j)
		iR=iR+n[0]
		iP=iP+n[1]
		iS=iS+n[2]
#	nextfield[i]=field[i];
#	nextfield[i]=field[i]+(influence[2]/STEPDIV-influence[1]/STEPDIV,influence[0]/STEPDIV-influence[2]/STEPDIV,influence[1]/STEPDIV-influence[0]/STEPDIV)
	nextfield[i]=(
		min(255,max(0,field[i][0]-iP/STEPDIV+iS/STEPDIV)),
		min(255,max(0,field[i][1]-iS/STEPDIV+iR/STEPDIV)),
		min(255,max(0,field[i][2]-iR/STEPDIV+iP/STEPDIV)))
#	if(nextfield[i][0]>nextfield[i][1] and nextfield[i][0]>nextfield[i][2]):
#		nextfield[i]=(nextfield[i][0],0,0)
#	if(nextfield[i][1]>nextfield[i][0] and nextfield[i][1]>nextfield[i][2]):
#		nextfield[i]=(0,nextfield[i][1],0)
#	if(nextfield[i][2]>nextfield[i][0] and nextfield[i][2]>nextfield[i][1]):
#		nextfield[i]=(0,0,nextfield[i][2])


def xy2i(x,y):
        if((y%2)==0):
                i=y*COLS+x
        else:
                i=y*COLS+(COLS-1-x)
        #print("{0},{1} = {2}".format(x,y,i))
        return i

while(1):
	for i in range(0,num_pixels):
		step(i)
		y=(int)(i/COLS)
		x=i%COLS
		pixels[xy2i(x,y)]=((int)(nextfield[i][0]),(int)(nextfield[i][1]),(int)(nextfield[i][2]))
	pixels.show()
	#time.sleep(1)
	field,nextfield = nextfield,field
