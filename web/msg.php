<?php 
function hue2rgb($im,$pos){
    # Input a value 0 to 255 to get a color value.
    # The colours are a transition r - g - b - back to r.
    if ($pos < 0 || $pos > 255){
        $r = $g = $b = 0;
    }elseif($pos < 85){
        $r = floor($pos * 3);
        $g = floor(255 - $pos*3);
        $b = 0;
    }elseif($pos < 170){
        $pos -= 85;
        $r = floor(255 - $pos*3);
        $g = 0;
        $b = floor($pos*3);
    }else{
        $pos -= 170;
        $r = 0;
        $g = floor($pos*3);
        $b = floor(255 - $pos*3);
    }
    return imagecolorallocate($im,$r, $g, $b);
}

$msg = $argv[1];
$brightness = $argv[2];
exec("../c/rawPlayer loading.raw 1 {$brightness}");
exec("rm -f msg.png; rm -f msg.ppm; rm -f msg.raw");
$font = getcwd()."/Ultrathins.ttf";
$size = 9;
$textsize = imagettfbbox($size,0,$font,$msg);
$textwidth = $textsize[2]-$textsize[0];
$textheight = $textsize[3]-$textsize[5];
$frames = 31+$textwidth;


$im = imagecreatetruecolor(31,16*$frames);

$black = imagecolorallocate($im,0,0,0);
imagefill($im,0,0,$black);
for($i=0;$i<$frames;$i++){
	$c = hue2rgb($im,$i%360);
	$x=31-$i;
	$y = 16/2 - $textheight/2 + 16*$i;
	imagettftext($im,$size,0,$x,$y,$c,$font,$msg);
}
imagepng($im,"msg.png");
exec("../c/rawPlayer processing.raw 1 {$brightness}");
exec("convert msg.png -compress none -set comment 'msg.php4LEDRGBTable' msg.ppm");
exec("python3 ../vid_conv/ppm2bytes.py msg.ppm msg.raw");
exec("../c/rawPlayer msg.raw 1 {$brightness}");

