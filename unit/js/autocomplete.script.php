<?php
set_include_path(get_include_path().':'.realpath(dirname(__FILE__).'/../../'));
require_once 'core/model/SyndService.class.inc';

class AutoCompleteService extends SyndXMLRPCService {
	function search($string) {
		$results = array();
		for ($i=rand(3,7); $i >= 0; $i--) {
			$id = substr(md5(uniqid('')),0,10);
			$results[$string.' '.$id] = '<span class="Suggestion">Mikael Johansson</span><span class="Matches">'.rand(1,100000).'</span>';
		}
		return $results;
	}
}

$service = new AutoCompleteService();
$service->run();

