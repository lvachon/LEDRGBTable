<?php
while(true){
	$cmd = file_get_contents("webcmd");
	if(!strlen($cmd)){sleep(0.25);continue;}
	if($cmd=="rand"){
		$a = glob("../raws/*.raw");
	        $file = $a[rand(0,floor(count($a)-1))];
	        $secs = 0.03 * filesize($file)/(31*16*3);
	        $loops = max(1,floor(30/$secs));
		$cmd = "../rawPlayer {$file} {$loops}";
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
