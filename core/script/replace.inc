#!/usr/bin/php
<?php
/**
 * Batch replacement script.
 *
 * Script takes a list of files on STDIN, read their content and 
 * applies a preg_replace()'ment on it. If changed it prints the
 * changes and if the '--force' flag is specified it will also write
 * the file back to disk.
 *
 * Handy when moving/renaming/refactoring methods/classes or changing 
 * parameter lists.
 *
 * @access		public
 * @package		synd.core
 */

set_error_handler('_synd_callback_error');

$force = false;
$candidates = 0;
$matched = 0;
$written = 0;

if (count($argv) < 2) {
	print "Usage: php replace.inc [PATTERN] [REPLACEMENT] [OPTION]...\n";
	print "List of files is read from stdin. Patterns will be sent to preg_replace()\n";
	print "See http://www.php.net/manual/en/reference.pcre.pattern.syntax.php for syntax help.\n";
	print "\n";
	print " Arguments\n";
	print "  --force	Write changed files back to disk.\n";
	print "  --find		Execute 'find' {.inc, .php, .tpl, ..} instead of reading files from stdin.\n";
	print "\n";
	print " Example.\n";
	print "  find . -name \*.inc | php core/script/replace.inc \"/tpl_link(\w*)\(/\" \"tpl_view\\\\1(\" --force\n";
	exit;
}

if (false !== ($key = array_search('--force', $argv))) {
	unset($argv[$key]);
	$argv = array_values($argv);
	$force = true;
}

if (false !== ($key = array_search('--find', $argv))) {
	if (false === ($stdin = popen('find . -name \*.inc -or -name \*.php -or -name \*.tpl -or -name \*.js -or -name \*.css -or -name \*.c -or -name \*.cs -or -name \*.sh','r')))
		return;
	unset($argv[$key]);
	$argv = array_values($argv);
	$isProcess = true;
}
else if (false === ($stdin = fopen('php://stdin','r')))
	return;


print "Pattern: $argv[1] Replacement: $argv[2]\n\n";
$macros = array('\n' => "\n", '\r' => "\r");

while (!feof($stdin) && null != ($file = trim(fgets($stdin)))) {
	unset($inBuffer, $outBuffer);
	$inBuffer = file_get_contents($file);
	$outBuffer = preg_replace($argv[1], strtr($argv[2],$macros), $inBuffer);
	
	if ($outBuffer != $inBuffer) {
		$matched++;
		$in = array_diff(explode("\n", $inBuffer), explode("\n", $outBuffer));
		$out = array_diff(explode("\n", $outBuffer), explode("\n", $inBuffer));
		
		print "$file: \n";
		foreach ($out as $linenum => $line) {
			print " ".($linenum+1).":\t".trim(array_shift($in));
			print "\n ".($linenum+1).":\t".trim($line)."\n";
		}
		print "\n";
		
		if (true == $force && false !== ($fp = fopen($file,'w')) && fwrite($fp, $outBuffer)) {
			$written++;
			fclose($fp);
		}
	}
	
	$candidates++;
}

if (!empty($isProcess))
	pclose($stdin);

print "\nMatched $matched files. Wrote $written files. Considered $candidates files.\n";

function _synd_callback_error($errno, $errstr, $errfile, $errline, $errcontext) {
	switch ($errno) {
		case E_ERROR:
		case E_USER_ERROR:
			print "Error: $errstr";
			break;
		case E_WARNING:
		case E_USER_WARNING:
			print "Warning: $errstr";
			break;
		case E_NOTICE:
		case E_USER_NOTICE:
			print "Notice: $errstr";
			break;
		default:
			print "Unknown ($errno): $errstr";
			break;
	}

	print " in $errfile on line $errline\n";
	exit;
}

