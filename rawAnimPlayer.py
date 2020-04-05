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

frame=1;
frames = (int)(len(animation)/(3*num_pixels))
while(1):
	neopixel_write.neopixel_write(pin,animation[3*num_pixels*frame-COLS:3*num_pixels*(frame+1)-COLS])
	frame=(frame+1)%frames
	time.sleep(0.01)

