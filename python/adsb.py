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
brightness = float(sys.argv[1])/255.0
print(brightness)
with open("neadsb.raw","rb") as file:
	animation = bytearray(file.read())
for i in range(0,len(animation)):
	animation[i]=int(animation[i]*brightness)
cleananim = bytearray(animation)
offset = COLS*ROWS*3
for frame in range(0,6):
	animation[:]=cleananim
	with urllib.request.urlopen('http://192.168.1.21:8080/data/aircraft.json') as response:
		res = response.read()
	print(res)
	planes = json.loads(res)["aircraft"]
	for plane in planes:
		try:
			x = int(COLS*(float(plane["lon"])-WEST)/(EAST-WEST))
			y = int(ROWS-ROWS*(float(plane["lat"])-SOUTH)/(NORTH-SOUTH))
			if((y%2)==1):
				x=COLS-1-x
			c = int(255*float(plane["alt_baro"])/45000)
			h = hue2rgb(c)
			animation[offset+3*(x+y*COLS)]=int(h[0]*brightness)
			animation[offset+3*(x+y*COLS)+1]=int(h[1]*brightness)
			animation[offset+3*(x+y*COLS)+2]=int(h[2]*brightness)
		except:
			continue
	neopixel_write.neopixel_write(pin,animation[offset:])
	time.sleep(5)
