<?php
require_once 'PHPUnit2/Framework/TestCase.php';

class _model_Type_Image extends PHPUnit2_Framework_TestCase {
	var $_width = 10;
	var $_height = 15;
	var $_quality = 50;
	var $_path = null;
	
	function setUp() {
		global $synd_config;
		$this->_path = $synd_config['dirs']['cache'].'_unit_test_'.md5(uniqid('')).'.jpg';
			
		$buffer = imagecreate($this->_width, $this->_height);
		imagejpeg($buffer, $this->_path, $this->_quality);
		$this->assertTrue(file_exists($this->_path));
		
		$size = getimagesize($this->_path);
		$this->assertEquals($this->_width, $size[0]);
		$this->assertEquals($this->_height, $size[1]);
	}
	
	function tearDown() {
		unlink($this->_path);
		$this->assertTrue(!file_exists($this->_path));
	}
	
	function testSize() {
		$image = SyndType::factory('image', $this->_path);
		
		$this->assertEquals($this->_width, $image->getWidth());
		$this->assertEquals($this->_height, $image->getHeight());
		
		$image->delete();
	}
	
	function testResize() {
		$image = SyndType::factory('image', $this->_path);
		
		$uri = $image->getResizedUri($this->_width - 5, $this->_height - 5);
		$size = getimagesize($uri);
		$this->assertEquals($this->_width - 5, $size[0]);
		$this->assertEquals($this->_height - 5, $size[1]);
		
		$actual = file_get_contents($uri);
		$this->assertTrue($actual);
		
		$image->delete();

		$actual = @file_get_contents($uri);
		$this->assertFalse($actual);
	}
	
	function testRotate() {
		$image = SyndType::factory('image', $this->_path);
		
		$image->rotate(90);
		$this->assertEquals($this->_width, $image->getHeight());
		$this->assertEquals($this->_height, $image->getWidth());

		$image->delete();
	}
}
