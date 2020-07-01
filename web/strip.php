<?php 
$url = $argv[1];
$brightness = $argv[2];

exec("convert \"{$url}\" -compress none -set comment 'strip.php4LEDRGBTable' strip.ppm");
exec("python3 ../vid_conv/ppm2bytes.py strip.ppm strip.raw");
exec("../c/rawPlayer strip.raw 1 {$brightness}");
file_put_contents("webcmd","../c/rawPlayer strip.raw 1 {$brightness}");
