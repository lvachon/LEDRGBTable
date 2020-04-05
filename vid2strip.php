<?php
$fname = $argv[1];
$COLS = 31;
$ROWS = 15;
//convert to frames
exec("ffmpeg -i {$fname} -vf scale={$COLS}:{$ROWS} -q:v 1 temp%5d.png");
//Assemble frames
$a = glob("temp*.png");
$frames = count($a);

$im = imagecreatetruecolor($COLS,$ROWS*$frames);
for($frame=0;$frame<$frames;$frame++){
	$in = imagecreatefrompng($a[$frame]);
	imagecopy($im,$in,0,$frame*$ROWS,0,0,$COLS,$ROWS);
}
$outname = explode(".",basename($fname))[0];
imagepng($im,"{$outname}_strip.png");
exec("magick convert {$outname}_strip.png -compress none {$outname}_strip.ppm");
exec("del temp*.png");
exec("python ppm2bytes.py {$outname}_strip.ppm {$outname}.raw");