<?php 
$url = $argv[1];
$brightness = $argv[2];
exec("../c/rawPlayer loading.raw 1 {$brightness}");
exec("rm -f img.ppm; rm -f img.raw");
exec("convert \"{$url}\" -thumbnail 31x16^ -gravity center -extent 31x16  -compress none img.ppm");
exec("../c/rawPlayer processing.raw 1 {$brightness}");
exec("python3 ../vid_conv/ppm2bytes.py img.ppm img.raw");
exec("../c/rawPlayer img.raw 1 {$brightness}");
file_put_contents("webcmd","");
