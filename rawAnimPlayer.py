#!/usr/bin/env python3
import board
import neopixel_write
import digitalio
import time
import math
import sys

pin = digitalio.DigitalInOut(board.D18)
pin.direction = digitalio.Direction.OUTPUT

COLS = 31
ROWS = 16
num_pixels = COLS*ROWS
#GRB ORDER

with open(sys.argv[1],"rb") as file:
	animation = bytearray(file.read())
while(1):
	frame=1;
	frames = (int)(len(animation)/(3*num_pixels))
	while(frame<frames):
		neopixel_write.neopixel_write(pin,animation[3*num_pixels*frame-COLS*3:3*num_pixels*(frame+1)-COLS*3])
		frame=(frame+1)
		time.sleep(0.01)
	if len(sys.argv)<3:
		break

