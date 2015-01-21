<?php
require_once 'PHPUnit2/Framework/TestCase.php';

class _modules_Language extends PHPUnit2_Framework_TestCase {
	function testLanguageDetection() {
		global $synd_config;
		if (count($synd_config['language']) < 2)
			$this->markTestSkipped();
			
		$checked = false;
		foreach (SyndLib::scanDirectory(dirname(__FILE__).'/_language') as $file) {
			if (null != ($expected = substr(SyndLib::chopExtension(basename($file)),0,2)) && isset($synd_config['language'][$expected])) {
				$checked = true;
				$text = file_get_contents($file);
				$actual = SyndLib::runHook('detect_locale', $text);
				$this->assertEquals($expected, $actual);
			}
		}
		
		if (!$checked)
			$this->markTestSkipped();
	}
}
