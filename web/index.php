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
				foreach($a as $file){
					$bn = basename($file,".raw");
					echo "<a href='?raw={$bn}'>{$bn}</a><br/>";
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




