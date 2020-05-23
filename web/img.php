<?php 
$url = $argv[1];
$brightness = $argv[2];
exec("../c/rawPlayer loading.raw 1 {$brightness}");
exec("rm -f img.png; rm -f img2.png; rm -f img.ppm; rm -f img.raw");
exec("convert \"{$url}\" -thumbnail 31x16^ -gravity center -extent 31x16 img.png");
exec("../c/rawPlayer processing.raw 1 {$brightness}");
$im = imagecreatetruecolor(31,32);
$a = imagecreatefrompng("img.png");
imagecopy($im,$a,0,0,0,0,31,16);
imagecopy($im,$a,0,15,0,0,31,16);
imagepng($im,"img2.png");
exec("convert img2.png -compress none -set comment 'img.php4LEDRGBTable' img.ppm");
exec("python3 ../vid_conv/ppm2bytes.py img.ppm img.raw");
exec("../c/rawPlayer img.raw 1 {$brightness}");
file_put_contents("webcmd","");
