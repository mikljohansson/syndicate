<?php
require_once 'core/model/type/task.class.inc';

class synd_type_unit_test_task extends synd_type_task {
	var $_file = null;
	
	function __construct($file) {
		parent::__construct();
		$this->_file = $file;
	}
	
	function run($handler) {
		print "Starting unittest task: {$this->_file}\n";
		flush();
		
		parent::run();
		SyndLib::file_put_contents($this->_file, 'Test');
		
		print "Stopping unittest task: {$this->_file}\n";
		flush();
	}
}
