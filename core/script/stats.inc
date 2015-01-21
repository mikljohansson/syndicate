#!/usr/bin/php
<?php
/**
 * Code statistics script
 *
 * Used to collect statistics on PHP and HTML code contained in a 
 * directory or showing the progress through multiple revisions in
 * an SVN repository.
 *
 * @access		public
 * @package		synd.core
 */

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

/**
 * @access	private
 */
function phpStatistics($dir) {
	return exec('grep -r -x "\s*[^\/*].*\w.*" '.escapeshellarg($dir).' | grep -v svn | grep .inc | wc -l');
}

/**
 * @access	private
 */
function htmlStatistics($dir) {
	return exec('grep -r -x ".*\w.*" '.escapeshellarg($dir).' | grep -v svn | grep .tpl | wc -l');
}

/**
 * @access	private
 */
function runCommand($cmd) {
	$spec = array(
		0 => array('pipe','r'),
		1 => array('pipe','w'),
		2 => array('pipe','w'));
	
	$output = null;
	$pd = proc_open($cmd, $spec, $pipes);
	
	if (is_resource($pd)) {
		fwrite($pipes[0],'');
		fclose($pipes[0]);
		
		while (!feof($pipes[1]))
			$output .= fgets($pipes[1], 1024);
		fclose($pipes[1]);

		while (!feof($pipes[2]))
			$output .= fgets($pipes[2], 1024);
		fclose($pipes[2]);
		
		proc_close($pd);
	}

	return $output;
}


$args = parseArguments($argv);
print "Lines of code (excluding comments and whitespace)\n";

if (empty($args['--from-rev'])) {
	$code = empty($args[1]) ? getcwd() : realpath($args[1]);
	if (empty($code))
		die("No source code directory specified.\n");

	print "PHP\t\tHTML\n";
	print phpStatistics($code)."\t\t";
	print htmlStatistics($code)."\n";
}
else {
	$temp = dirname(getcwd()).'/.stats';
	if (file_exists($temp))
		die("Working directory '$temp' exists, remove and run script again.\n");

	$phpLast = null;
	$htmlLast = null;
	
	$stride = empty($args['--stride']) ? 1 : $args['--stride'];
	$from = $args['--from-rev'];
	$prev = $from;
	$to = empty($args['--to-rev']) ? 'HEAD' : $args['--to-rev'];

	if (!empty($args['--uri'])) 
		$uri = $args['--uri'];
	else if (!empty($args['--url'])) 
		$uri = $args['--url'];
	else {
		$info = shell_exec('svn info '.escapeshellarg($code));
		if (!preg_match('/URL: (.+)/', $info, $matches))
			die("Could not get url to repository from working copy at '$temp'.\n");
		$uri = $matches[1];
	}

	mkdir($temp);
	exec('svn checkout '.escapeshellarg($uri).' '.escapeshellarg($temp).' --revision '.escapeshellarg($from));

	print "Revision  Date        PHP            HTML\n";
	for ($break=false; ($from <= $to || 'HEAD' == $to) && !$break; $from += $stride) {
		// Switch to revision
		$output = runCommand('svn switch '.escapeshellarg($uri).' '.escapeshellarg($temp).' --revision '.escapeshellarg($from));
		if (false !== strpos($output, 'non-existent revision')) {
			exec('svn switch '.escapeshellarg($uri).' '.escapeshellarg($temp).' --revision HEAD');
			$break = true;
			$from = 'HEAD';
		}
		
		printf('%-10s', $from);

		$info = shell_exec('svn info '.escapeshellarg($temp));
		preg_match('/Last Changed Date: ([\d\-]+)/', $info, $matches);
		printf('%-12s', $matches[1]);

		$php = phpStatistics($temp);
		$html = htmlStatistics($temp);
		
		printf('%-15s', $php.sprintf('%+d', $php-$phpLast));
		printf('%-15s', $html.sprintf('%+d', $html-$htmlLast));
		print "\n";

		$phpLast = $php;
		$htmlLast = $html;
	}

	exec('rm -rf '.escapeshellarg($temp));
}

