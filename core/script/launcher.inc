<?php
/**
 * Task process launcher.
 *
 * Script is used by the task handler to launch new worker 
 * processes. Log messages is written to STDOUT.
 *
 * @access		public
 * @package		synd.core
 */

if (count($argv) < 2) {
	print "Usage: php launcher.inc [TASK FILE] [INCLUDE PATH] [MAX EXECUTION TIME] [HANDLER ID]\n";
	print "Takes the path to a serialized task object file which is unserialized \n";
	print "before invoking run() on the resulting object.\n";
	exit;
}

set_include_path($argv[2]);
set_time_limit($argv[3]);

require_once 'synd.inc';
require_once 'core/lib/SyndTaskHandler.class.inc';

ini_set('display_startup_errors', true);
ini_set('display_errors', true);
error_reporting(E_ALL);
ob_implicit_flush(1);

print "Launcher: Running (pid ".posix_getpid().")\n";
if (isset($argv[1]) && file_exists($argv[1])) {
	print "Launcher: Running task: {$argv[1]}\n";
	$info = unserialize(file_get_contents($argv[1]));

	if (null == ($task = unserialize($info['task']))) 
		print "Launcher: Failed to unserialize task\n";
	else {
		$ts = time();
		$status = $task->run(new SyndTaskHandler($argv[4]));
		print "Launcher: Task done in ".(time()-$ts)." seconds\n";
	}
}

exit((int)$status);
