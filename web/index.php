<?php
$brightness = intval($_GET['brightness']);
if(isset($_GET['raw'])){
	$file = "../raws/{$_GET['raw']}.raw";
        $secs = 0.03 * filesize($file)/(31*16*3);
        $loops = max(1,floor(30/$secs));
	$fname = escapeshellarg ( "../raws/".$_GET['raw'].".raw" );
	$cmd = "../c/rawPlayer {$fname} {$loops} {$brightness}";
	file_put_contents("webcmd",$cmd);
	die();
}
if(isset($_GET['rps'])){
	$cmd = "../c/rps 1 {$brightness}";
	file_put_contents("webcmd",$cmd);
	die();
}
if(isset($_GET['black'])){
	$cmd = "../python/black.py";
	file_put_contents("webcmd",$cmd);
	die();
}
if(isset($_GET['matrix'])){
	$cmd = "../c/matrix 1 {$brightness}";
	file_put_contents("webcmd",$cmd);
	die();
}
if(isset($_GET['clock'])){
	$cmd = "cd ../c/;./clock 1 {$brightness}";
	file_put_contents("webcmd",$cmd);
	die();
}
if(isset($_GET['life'])){
	$cmd = "cd ../c/;./life 1 {$brightness}";
	file_put_contents("webcmd",$cmd);
	die();
}
if(isset($_GET['rand'])){
	$a = glob("../raws/*.raw");
	$file = $a[rand(0,floor(count($a)-1))];
	$secs = 0.03 * filesize($file)/(31*16*3);
	$loops = max(1,floor(30/$secs));
	$cmd = "rand {$brightness}";
	file_put_contents("webcmd",$cmd);
	die();
}?>
<html>
	<head>
		<style>
			.vid {margin:2px;border:solid 1px #888888;height:2.5cm;}
			body {background:#000000;color:#AAAAAA;font-weight:bold;font-family:sans-serif;}
			a {color:#EEEEEE;font-weight:bold;font-size:2.5cm;}
			.status {font-size:1cm;}
		</style>
		<script>
			function playVid(vid){
				fetch(`index.php?raw=${vid}&brightness=${brightness.value}`).then((response) => {
					return response.text();
				}).then((data) => {
					nextcmd.innerHTML=vid;
				});
			}
			function runCmd(cmd){
				fetch(`index.php?${cmd}&brightness=${brightness.value}`).then((response) => {
					return response.text();
				}).then((data) => {
					nextcmd.innerHTML=cmd;
				});
			}
		</script>
	</head>
	<body>
		<h1>LEDRGBTable</h1>
		<div class='status'>
			Next command: <span id='nextcmd'><?php echo file_get_contents("webcmd");?></span><br/>
			Current command: <span id='curcmd'><?php echo file_get_contents('curcmd');?></span>
		</div>
		<div><input value='255' type='numeric' id='brightness' maxval='255' minval='0'/></div>
		<div>	
			<h2>Vids</h2>
			<?php 
				$a = glob("../raws/*.raw");
				$im = imagecreatetruecolor(31,16);
				foreach($a as $file){
					$f = fopen($file,"rb");
					$frames = filesize($file)/(31*16*3);
					$offset = floor($frames/2)*(31*16*3)-31*3;
					fseek($f,$offset);
					for($i=0;$i<31*16;$i++){
						$y=floor($i/31);
						$x=$i%31;
						if(($y%2)){$x=30-$x;}
						$g = ord(fgetc($f));
						$r = ord(fgetc($f));
						$b = ord(fgetc($f));
						$c = imagecolorallocate($im,$r,$g,$b);
						imagesetpixel($im,$x,$y,$c);
					}
					fclose($f);
					ob_flush();
					ob_start();
					imagepng($im);
					$img = ob_get_clean();
					$bn = basename($file,".raw");
					echo "<img onclick='playVid(\"{$bin}\");' class='vid' src='data:image/png;base64, ".base64_encode($img)."'/>";
				}

			?>
			<a href='javascript:runCmd("rand=1");'>[ ? ]</a>
		</div>
		<div>
			<h2>Progs</h2>
			<a href='javascript:runCmd("rps=1")'>RPS</a><br/>
			<a href='javascript:runCmd("life=1")'>Life</a><br/>
			<a href='javascript:runCmd("matrix=1")'>Matrix</a><br/>
			<a href='javascript:runCmd("clock=1")'>Clock</a><br/>
			<a href='javascript:runCmd("black=1")'>Off</a>
		</div>
	</body>
	<script>
		function getCurcmd(){
			fetch(`curcmd?${Math.random()}`).then((response) => {
				return response.text();
			}).then((data) => {
				curcmd.innerHTML=data;
				setTimeout(getCurcmd,2000);
			});

		}
		getCurcmd();
	</script>
</html>





