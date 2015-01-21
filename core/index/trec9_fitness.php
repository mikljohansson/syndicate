#!/usr/bin/php
<?php
require_once dirname(__FILE__).'/../../local/synd.inc';
require_once 'core/index/plugin/stem.class.inc';
require_once 'core/index/plugin/text.class.inc';
require_once 'core/index/SyndGeneticOptimizer.class.inc';
require_once 'core/index/IndexQuery.class.inc';

if (isset($_SERVER['REMOTE_ADDR']))
	die('This script must be run from commandline.');

if (count($argv) < 2) {
	print "Usage: php {$argv[0]} /path/to/queries /path/to/queryresults /path/to/database IndexingScheme\n";
	print " Example.\n";
	print "  php {$argv[0]} ohsu-trec/trec9-train/query.ohsu.1-63 ohsu-trec/trec9-train/qrels.ohsu.batch.87 mysql://user:password@localhost/search term\n";
	print "  php {$argv[0]} ohsu-trec/trec9-train/query.ohsu.1-63 ohsu-trec/trec9-train/qrels.ohsu.batch.87 /home/search/xapian xapian\n";
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

if ('xapian' != $clsid)
	require_once 'core/index/plugin/bm25.class.inc';
	$index->loadExtension(new synd_plugin_bm25());
}

$trec9 = new Trec9IndexEvaluator($argv[1], $argv[2]);
$evaluator = new SyndFitnessEvaluator($index, $trec9);
$evaluator->run(50,1500);

