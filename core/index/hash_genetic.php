<?php
require_once dirname(__FILE__).'/../../local/synd.inc';
require_once 'core/index/SyndGeneticEvaluator.class.inc';

if (isset($_SERVER['REMOTE_ADDR']))
	die('This script must be run from commandline.');

set_time_limit(0);
error_reporting(E_ALL);

global $synd_maindb;

$sql = "
	SELECT w.content FROM synd_search_webpage w
	WHERE w.uri = 'http://www.mot.chalmers.se/dept/ima/examensarbeten/PDF/SwedishComp.pdf'";
$content = gzuncompress($synd_maindb->getOne($sql));
preg_match_all('/\w+|\d+/', $content, $matches);

require_once 'core/lib/SyndDLE.class.inc';
SyndDLE::functionExistsDL('synd_stem_en', 'stem_en');
$terms = array_map('synd_stem_en', $matches[0]);

$evaluator = new HashFunctionEvaluator($terms);

$genetic = new SyndGeneticEvaluator($evaluator);
$genetic->run(1000);
