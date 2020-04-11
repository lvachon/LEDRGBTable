<?php
while(true){
	$cmd = file_get_contents("webcmd");
	if(!strlen($cmd)){sleep(0.25);continue;}
	echo("Executing {$cmd}...");
	exec($cmd);
	file_put_contents("webcmd","");
	echo("Done..");
}
