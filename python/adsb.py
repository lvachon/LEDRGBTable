#!/usr/bin/env python3
import board
import neopixel_write
import digitalio
import time
import math
import sys
import urllib.request
import json

pin = digitalio.DigitalInOut(board.D18)
pin.direction = digitalio.Direction.OUTPUT


COLS = 31
ROWS = 16
NORTH = 43.4693
SOUTH = 40.4541
EAST = -68.2321
WEST = -74.0321
#num_pixels = COLS*ROWS
#ORDER = neopixel.GRB
#pixels = neopixel.NeoPixel(pixel_pin, num_pixels, brightness=1.0, auto_write=False, pixel_order=ORDER)
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
    return (r, g, b)

with open("neadsb.raw","rb") as file:
	animation = bytearray(file.read())
cleananim = bytearray(animation)
for frame in range(0,30):
	animation[:]=cleananim
	with urllib.request.urlopen('http://192.168.1.21:8080/data/aircraft.json') as response:
		res = response.read()
	print(res)
	planes = json.loads(res)["aircraft"]
	for plane in planes:
		try:
			x = int(COLS*(float(plane["lon"])-WEST)/(EAST-WEST))
			y = int(ROWS-ROWS*(float(plane["lat"])-SOUTH)/(NORTH-SOUTH))
			if(y%2):
				x=COLS-1-x
			c = int(255*float(plane["alt_baro"])/45000)
			h = hue2rgb(c)
			animation[x+y*COLS]=h[0]
			animation[x+y*COLS+1]=h[1]
			animation[x+y*COLS+2]=h[2]
		except:
			continue
	neopixel_write.neopixel_write(pin,animation)
	time.sleep(5)
