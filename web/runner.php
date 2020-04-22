<?php
while(true){
	$cmd = file_get_contents("webcmd");
	if(!strlen($cmd)){echo("SLEEP\n");sleep(1);continue;}
	if(substr($cmd,0,4)=="rand"){
		$brightness = intval(explode(" ",$cmd)[1]);
		$a = glob("../raws/*.raw");
	        $file = $a[rand(0,floor(count($a)-1))];
	        $secs = 0.03 * filesize($file)/(31*16*3);
	        $loops = max(1,floor(30/$secs));
		$cmd = "../c/rawPlayer {$file} {$loops} {$brightness}";
	}
	echo(date("M-d-Y H:i:s")." Executing {$cmd}...\n");
	file_put_contents("curcmd",$cmd);
	exec($cmd);
	if($cmd=="../python/black.py"){
		file_put_contents("webcmd","");
	}
	echo(date("M-d-Y H:i:s")." Done..\n");
	sleep(0.25);
}
