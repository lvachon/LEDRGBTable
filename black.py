#!/usr/bin/env python
import board
import neopixel
#import time
#import math
pixel_pin = board.D18
COLS = 31
ROWS = 11
num_pixels = COLS*ROWS

ORDER = neopixel.GRB

pixels = neopixel.NeoPixel(pixel_pin, num_pixels, brightness=0.25, auto_write=False, pixel_order=ORDER)

for i in range(0,num_pixels):
	pixels[i]=0;
pixels.show()
