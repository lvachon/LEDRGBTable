#!/usr/bin/env python3
import subprocess
import random

random.seed()

anims = (
	('boxes.raw','8'),
	('esheep.raw','1'),
	('honeyhole.raw','8'),
	('ogprpsp.raw','0'),
	('plasma.raw','0'),
	('rainbowswirl.raw','8'),
	('rainbowwiggle.raw','8'),
	('rgbrpsp.raw','0'),
	('rings.raw','8'),
	('siezureboxes.raw','8'),
	('sohotrightnow.raw','8'),
	('spiralz.raw','8'),
	('twirly.raw','8')
)



while(1):
	index = random.randrange(len(anims))
	cmd = ('./rawAnimPlayer.py',anims[index][0],anims[index][1])
	print(cmd)
	subprocess.run(cmd)
