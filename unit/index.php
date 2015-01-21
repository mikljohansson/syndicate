<?php
require_once 'synd.inc';
require_once 'core/lib/Template.class.inc';
require_once 'HtmlUI/TestRunner.php';
require_once 'PHPUnit2/Framework/TestSuite.php';
require_once 'PHPUnit2/Framework/TestCase.php';

if (!isset($_SERVER['REMOTE_ADDR']) || !isset($synd_config['debug_allowed_ips'][$_SERVER['REMOTE_ADDR']])) 
	throw new ForbiddenException(SyndLib::text("Please add your ip address '%s' to the \$synd_config['debug_allowed_ips'] array in 'synd.inc' to enable unit test access.", $_SERVER['REMOTE_ADDR']));

ignore_user_abort(0);
set_time_limit(5);

$dirs = array_merge(
	array(dirname(__FILE__)), 
	isset($synd_config['dirs']['unit']) ? $synd_config['dirs']['unit'] : array());

$runner = new PHPUnit2_HtmlUI_TestRunner();
$runner->start($dirs, $_SERVER, $_REQUEST);
