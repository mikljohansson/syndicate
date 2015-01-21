<?php
/**
 * Load tester
 *
 * Script is used by the task handler to launch new worker 
 * processes. Log messages is written to STDOUT.
 *
 * @access		public
 * @package		synd.core
 */

if (count($argv) < 2) {
	print "Usage: php load.inc [OPTION] [FILE]\n";
	print "Transaction throughput tester\n";
	print "\n";
	print "  -j, --jobs		Allow N paralell processes, default 2\n";
	print "  -l, --load		Average load to maintain, default 1\n";
	exit;
}

/**
 * @access	private
 */
function parseArguments($args) {
	$result = array(0 => $args[0]);
	$j = 1;
	
	for ($i=1; $i<count($args); $i++) {
		if ('-' == substr($args[$i],0,1) && isset($args[$i+1])) {
			$result[$args[$i]] = $args[$i+1];
			$i++;
		}
		else
			$result[$j++] = $args[$i];
	}
	
	return $result;
}

$args = parseArguments($argv);
$jobs = isset($args['-j']) ? $args['-j'] : (isset($args['--jobs']) ? $args['--jobs'] : 2);
$load = isset($args['-l']) ? $args['-l'] : (isset($args['--load']) ? $args['--load'] : 0.75);
$pid = true;

for ($i=1; $i<$jobs; $i++) {
	if (0 == ($pid = pcntl_fork()))
		break;
}

set_include_path(get_include_path().':'.realpath(dirname(__FILE__).'/../../'));
include_once 'synd.inc';

assert_options(ASSERT_BAIL, true);
assert_options(ASSERT_ACTIVE, true);
assert_options(ASSERT_WARNING, true);

if (!(include_once(end($args))))
	trigger_error("Could not open task file '".end($args)."'", E_USER_ERROR);
if (!isset($transaction))
	trigger_error("\$transaction object not defined by file '".end($args)."'", E_USER_ERROR);

$count = 0;
$start = time();
$sleep = 500000*$jobs;
$avg = 0;

if ($pid)
	print "$jobs jobs, average load $load.\n";

//profiler_start();

$t1 = time();
$t2 = time();
$lastLoadavg = null;

$ts1 = microtime();
$lastCount = 0;

for (;; $count++) {
	$tts1 = microtime();
	$transaction->run();
	$tts2 = microtime();

	$ats1 = explode(' ', $tts1);
	$ats2 = explode(' ', $tts2);
	$tts = $ats2[0] + $ats2[1] - $ats1[0] - $ats1[1];
	
	usleep($sleep);
	$sleep = max(1, $sleep + $sleep * ($tts - $load) * 0.01);
	
	if ($pid && time() > $t1+5) {
		$ts2 = microtime();

		$ats1 = explode(' ', $ts1);
		$ats2 = explode(' ', $ts2);
		$stts = $ats2[0] + $ats2[1] - $ats1[0] - $ats1[1];

		printf("Pageload %.3f, sleep %d usecs, %d transactions, %d TPM avg total, %d TPM sliding avg\n", 
			$tts, $sleep, $count*$jobs, $count/(time()-$start)*60*$jobs, ($count-$lastCount)/$stts*60*$jobs);

		$lastCount = $count;
		$ts1 = microtime();
		$t1 = time();
	}

//	if (time() > $t2+20) {
//		$t2 = time();
//		print "\n\n";
//		$GLOBALS['_profiler_instance']->display();
//		print "\n\n";
//	}
}
