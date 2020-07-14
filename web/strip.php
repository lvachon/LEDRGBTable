<?php 
$url = $argv[1];
$brightness = $argv[2];

//exec("convert \"{$url}\" -compress none -set comment 'strip.php4LEDRGBTable' strip.ppm");
//exec("python3 ../vid_conv/ppm2bytes.py strip.ppm strip.raw");
file_put_contents("strip.raw",file_get_contents($url));
$frames = filesize("strip.raw")/(3*31*16);
$loops = floor(900/$frames);
exec("../c/rawPlayer strip.raw {$loops} {$brightness}");
//file_put_contents("webcmd","../c/rawPlayer strip.raw 1 {$brightness}");
