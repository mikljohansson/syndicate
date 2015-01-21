#!/usr/bin/php
<?php
require_once dirname(__FILE__).'/../../local/synd.inc';
require_once 'core/global.inc';

set_time_limit(0);
ignore_user_abort(0);
error_reporting(E_ALL);

$search = Module::getInstance('search');
$index = $search->getWebIndex();
//$index->begin();
//$index->delDocument('http://www.chalmers.se');

print $index->getDocumentCount();

$queue = new SyndBufferedSpiderQueue(new SyndWebSpiderQueue($index->_db, $index));
$queue->_queue[] = 'http://www.chalmers.se';

$filter = new SyndSpiderURIFilter(
	array(
//		"/^https?:\/\/([\w.]+\.)?{$_SERVER['SERVER_NAME']}(:\d+)?($|\/.*)/i",
		"/^https?:\/\/[\w\.@:]*chalmers.se/i",
		"/^https?:\/\/[\w\.@:]*ituniv.se/i",
		),
	array(
		"/^https?:\/\/[\w\.\-]+(\/\w*)*(\?.*)?$/i",
		'/(\/|\.(html?|php|jsp|asp|cfm|xml|pdf|doc|xls|ppt|sxw))(\?.*)?$/iS',
		),
	array(
		'/\.(css|rss|jpe?g|js|gif|png|ico|sit|eps|wmf|zip|mpg|gz|rpm|tgz|mov|mov|exe)(\?.*)?$/i',
		),
	array(
		'/^text\/(html|plain)\b/i',
		'/^application\/(pdf|msword|msexcel|vnd\.ms-excel|vnd\.ms-powerpoint|vnd\.sun\.xml\.writer|x-soffice)\b/i'
		),
	array(
		'/(\?|&amp;|&)PHPSESSID=\w{32}(&amp;|&)?/S' => '\1',
		'/[?&]$/S' => '',
		));

$spider = new SyndSpider($queue, $index);
$spider->loadExtension($filter);

$spider->run();

$index->commit();
$index->analyze();
