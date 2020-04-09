#!/usr/bin/env python3
import subprocess
import random

random.seed()

anims = (
	('raws/boxes.raw','8'),
	('raws/esheep.raw','1'),
	('raws/honeyhole.raw','8'),
	('raws/ogprpsp.raw','0'),
	('raws/plasma.raw','0'),
	('raws/rainbowswirl.raw','8'),
	('raws/rainbowwiggle.raw','8'),
	('raws/rgbrpsp.raw','0'),
	('raws/rings.raw','8'),
	('raws/siezureboxes.raw','8'),
	('raws/sohotrightnow.raw','8'),
	('raws/spiralz.raw','8'),
	('raws/twirly.raw','8')
)



while(1):
	index = random.randrange(len(anims))
	cmd = ('./rawPlayer',anims[index][0],anims[index][1])
	print(cmd)
	subprocess.run(cmd)
