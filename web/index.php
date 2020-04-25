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
if(isset($_GET['warp'])){
	$cmd = "cd ../c/;./warp 1 {$brightness}";
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
			.vid {margin:0.1cm;border:solid 1px #888888;height:2.5cm;}
			body {background:#000000;color:#AAAAAA;font-weight:bold;font-family:sans-serif;font-size:1cm;}
			a {color:#EEEEEE;font-weight:bold;font-size:2.5cm;}
			.status {font-size:0.75cm;}
			.oflow {width:100%;height:40%;overflow-y:auto;text-align:center;}
			button {background:#404040;color:#AAAAAA;width:5cm;height:2.5cm;font-size:1cm;margin-bottom:0.1cm;}
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
		<div style='font-size:1cm;'>
			<label>
				Brightness:
				<input value='127' type='range' step='1' id='brightness' min='0' max='255' width='80%' onchange='btxt.value=`%${Math.floor(this.value/2.5)}`;'/>
			</label>
			<span id='btxt'>50%</span>
		</div>
		<div>	
			<h2>Vids</h2>
			<div class='vidbox oflow'>
				<?php 
					$a = glob("../raws/*.raw");
					$im = imagecreatetruecolor(31,16);
					foreach($a as $file){
						$bn = basename($file,".raw");
						if(!file_exists("../raws/png/{$bn}.png")){
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
							imagepng($im,"../raws/png/{$bn}.png");
						}
						echo "<img onclick='playVid(\"{$bn}\");' class='vid' src='rawpng/{$bn}.png' alt='{$bn}'/>";
					}

				?>
				<button onclick='runCmd("rand=1");' style='font-size:2cm;'>?</button>
			</div>
		</div>
		<div>
			<h2>Progs</h2>
			<div class='progbox oflow'>
				<button onclick='runCmd("rps=1")'>RPS</button>
				<button onclick='runCmd("warp=1")'>Warp</button>
				<button onclick='runCmd("life=1")'>Life</button>
				<button onclick='runCmd("matrix=1")'>Matrix</button>
				<button onclick='runCmd("clock=1")'>Clock</button>
				<button onclick='runCmd("black=1")'>Off</button>
			</div>
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





