#!/usr/bin/php
<?php
require_once dirname(__FILE__).'/../../local/synd.inc';
require_once 'core/index/plugin/stem.class.inc';
require_once 'core/index/plugin/text.class.inc';

if (isset($_SERVER['REMOTE_ADDR']))
	die('This script must be run from commandline.');

if (count($argv) < 2) {
	print "Usage: php {$argv[0]} /path/to/oshumed /path/to/database IndexingScheme\n";
	print " Example.\n";
	print "  php {$argv[0]} ohsu-trec/trec9-train/ohsumed.87 mysql://user:password@localhost/search term\n";
	print "  php {$argv[0]} ohsu-trec/trec9-train/ohsumed.87 /home/search/xapian xapian\n";
	exit;
}

set_time_limit(0);
error_reporting(E_ALL);

$dsn = isset($argv[2]) ? $argv[2] : 'mysql://root@localhost/search';
$clsid = isset($argv[3]) ? $argv[3] : 'xapian';
$class = "synd_index_$clsid";

require_once "core/index/driver/$clsid.class.inc";
$index = new $class($dsn);

$index->loadExtension(new synd_plugin_stem('en'));
$index->loadExtension(new synd_plugin_text());

$index->clear();
$index->begin();

$ok = 0;
$fail = 0;
$ts = time();

if (false == ($fp = fopen($argv[1],'r')))
	exit(1);

while (!feof($fp)) {
	$line = explode(' ',fgets($fp, 4096));
	if ('.I' == $line[0]) {
		if (isset($data['id'])) {
			$document = $index->createComposite(array(
				$index->createFragment($data['terms']),
				$index->createFragment($data['title']),
				$index->createFragment($data['abstract'],0.8)));
			$index->addDocument($data['pageid'], $document);
			
			unset($document);
			unset($data);

			$data = array(
				'terms' => null,
				'title' => null,
				'abstract' => null,
				);
			
			
			if (0 == $ok%1000)
				print $ok;
			if (0 == $ok%100)
				print '.';
				
			$ok++;
		}
		$data['id'] = trim($line[1]);
	}
	else {
		$text = trim(fgets($fp, 4096));
		switch (trim($line[0])) {
			case '.U':
				$data['pageid'] = $text;
				break;
			case '.M':
				$data['terms'] = $text;
				break;
			case '.T':
				$data['title'] = $text;
				break;
			case '.P':
				$data['type'] = $text;
				break;
			case '.W':
				$data['abstract'] = $text;
				break;
			case '.A':
				$data['author'] = $text;
				break;
			case '.S':
				$data['source'] = $text;
				break;
		}
	}
}

fclose($fp);

print "\nCommiting index\n";
$index->commit();

print "Inserted $ok rows\n";
print "Failed $fail rows\n";

print "Analyzing index\n";
$index->analyze();

print "Duration: ".tpl_duration(time()-$ts)."\n";

