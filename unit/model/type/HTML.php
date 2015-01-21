<?php
require_once 'PHPUnit2/Framework/TestCase.php';

class _model_Type_HTML extends PHPUnit2_Framework_TestCase {
	var $x = 10, $y = 10, $q = 50;
	var $file = null;
	var $html = null;
	var $image = null;

	function setUp() {
		global $synd_config;
		$this->file = $synd_config['dirs']['cache'].'_unit_type_html.jpg';

		$hImg = imagecreate($this->x, $this->y);
		imagejpeg($hImg, $this->file, $this->q);
		$this->assertTrue(file_exists($this->file), 'create testfile');
		$this->html = SyndType::factory('html');

		$this->image = SyndType::factory('image', $this->file);
		$this->html->_list[] = $this->image;
	}
	
	function tearDown() {
		// Teardown
		SyndLib::unlink($this->file);
		$this->assertTrue(!file_exists($this->file), 'delete testfile');
		$this->html->delete();
	}
	
	function testImportText() {
		$buf = 'bar';
		$this->html->importText($buf);
		$this->assertEquals($buf, $this->html->toString());
	}

	function testResizeImgAttrib() {
		// Test resize with img attrib
		$uri = $this->image->uri();
		$uri2 = $this->image->getResizedUri(5, 5);
		
		$buf = "<img src=\"$uri\" width=\"5\" height=\"5\" />";
		$this->html->importText($buf);
		
		$this->assertTrue(false === strpos($this->html->toString(), $uri), 'img attrib: ');
		$this->assertTrue(false !== strpos($this->html->toString(), $uri2), 'img attrib: ');
	}
	

	function testResizeImgStyle() {
		$uri = $this->image->uri();
		$uri2 = $this->image->getResizedUri(7, 7);
		
		$buf = "<img src=\"$uri\" style=\"width:7px; height:7px;\" />";
		$this->html->importText($buf);
		
		$this->assertTrue(false !== strpos($this->html->toString(), $uri2), 'img style: ');
	}

	function testClone() {
		$uri2 = $this->image->getResizedUri(5, 5);
		$buf = "<img src=\"$uri2\" style=\"width: 7px; height: 7px\" />";
		$this->html->importText($buf);

		// Test clone
		$clone = clone $this->html;
		$type2 = $clone->_list[0];
		
		$this->assertTrue(is_object($type2), 'no image cloned ');
		$this->assertTrue($this->image->uri() != $type2->uri(), 'cloned image is shallow copy ');
		$this->assertTrue(false !== strpos($clone->toString(), $type2->getResizedUri(7,7)), 'cloned image uri\'s not replaced in text ');

		$clone->delete();
	}

	function testDeleteUnusedImg() {
		// Test delete of unused images
		$uri = $this->image->uri();
		$buf = "<img src=\"$uri\" />";
		$this->html->importText($buf);

		$buf = 'bar';
		$file = $this->image->path();
		$this->html->importText($buf);
		
		$this->assertTrue(!file_exists($file), 'delete unused file failed ');
		if (file_exists($file))
			$this->image->delete();
	}
}

?>