<?php
if(isset($_GET['raw'])){
	$file = "../raws/{$_GET['raw']}.raw";
        $secs = 0.03 * filesize($file)/(31*16*3);
        $loops = max(1,floor(60/$secs));
	$fname = escapeshellarg ( "../raws/".$_GET['raw'].".raw" );
	$cmd = "../rawPlayer {$fname} {$loops}";
	file_put_contents("webcmd",$cmd);
}
if(isset($_GET['rps'])){
	$loops = intval($_GET['loops']);
	$cmd = "../rps {$loops}";
	file_put_contents("webcmd",$cmd);
}
if(isset($_GET['black'])){
	$cmd = "../python/black.py";
	file_put_contents("webcmd",$cmd);
}
if(isset($_GET['rand'])){
	$a = glob("../raws/*.raw");
	$file = $a[rand(0,floor(count($a)-1))];
	$secs = 0.03 * filesize($file)/(31*16*3);
	$loops = max(1,floor(60/$secs));
	$cmd = "../rawPlayer {$file} {$loops}";
	file_put_contents("webcmd",$cmd);
}?>
<html>
	<body>
		<h1>LEDREGTable</h1>
		<div>	
			<h2>Vids</h2>
			<?php 
				$a = glob("../raws/*.raw");
				$im = imagecreatetruecolor(31,16);
				foreach($a as $file){
					$f = fopen($file,"rb");
					for($i=0;$i<31*16;$i++){
						$y=floor($i/31);
						$x=$i%31;
						if(!($y%2)){$x=30-$x;}
						$r = fread($f,1);
						$g = fread($f,1);
						$b = fread($f,1);
						$c = imagecolorallocate($im,$r,$g,$b);
						imagesetpixel($im,$x,$y,$c);
					}
					fclose($f);
					ob_flush();
					ob_start();
					imagepng($im);
					$img = ob_get_clean();
					$bn = basename($file,".raw");
					echo "<a href='?raw={$bn}'>{$bn}<img src='data:image/png;base64, ".base64_encode($img)."'/></a><br/>";
				}

			?>
			<a href='?rand=1'>Random vid</a>
		</div>
		<div>
			<h2>Progs</h2>
			<a href='?rps=1'>Rock Paper Scissors</a><br/>
			<a href='?black=1'>Black</a>
		</div>
	</body>
</html>




