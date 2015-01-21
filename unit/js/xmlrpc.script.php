<?php
set_include_path(get_include_path().':'.realpath(dirname(__FILE__).'/../../'));
require_once 'core/model/SyndService.class.inc';

class UnitTestService extends SyndXMLRPCService {
	function echoParameter($string) {
		return $string;
	}
}

$service = new UnitTestService();
$service->run();
