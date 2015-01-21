<?php
require_once 'PHPUnit2/Framework/TestCase.php';

class _model_Type_File extends PHPUnit2_Framework_TestCase {
	var $_path = null;
	var $_content = null;
	
	function setUp() {
		global $synd_config;
		$this->_path = $synd_config['dirs']['cache'].'_unit_test';
		$this->_content = md5(uniqid(''));

		SyndLib::file_put_contents($this->_path, $this->_content);
		$this->assertTrue(file_exists($this->_path));
	}
	
	function tearDown() {
		SyndLib::unlink($this->_path);
		$this->assertFalse(file_exists($this->_path));
	}
	
	function testImport() {
		$file = SyndType::factory('file', $this->_path, 'file.txt');
		$this->assertEquals($this->_content, file_get_contents($file->path()));
		$this->assertEquals('file.txt', basename($file->path()));
		$this->assertTrue(false === strpos($file->nodeId(),'/'));
		$file->delete();
	}	

	function testDelete() {
		$file = SyndType::factory('file', $this->_path, 'file.txt');
		$path = $file->path();
		$this->assertEquals($this->_content, file_get_contents($path));

		$file->delete();
		$this->assertFalse(@file_get_contents($path));
	}
	
	function testClone() {
		$file = SyndType::factory('file', $this->_path, 'file.txt');
		$clone = clone $file;
		
		$this->assertEquals($this->_content, file_get_contents($file->path()));
		$this->assertEquals($this->_content, file_get_contents($clone->path()));
		$this->assertEquals('file.txt', basename($clone->path()));
		$this->assertTrue($file->path() != $clone->path());
		
		$file->delete();
		$clone->delete();
	}
	
	function testRewrite() {
		SyndType::loadClass('file');
		$this->assertEquals('file.txt', synd_type_file::_rewrite('file.txt'));	
		$this->assertEquals('.htaccess.txt', synd_type_file::_rewrite('.htaccess'));	
		$this->assertEquals('file.php.txt', synd_type_file::_rewrite('file.php'));	
		$this->assertEquals('file.php3.txt', synd_type_file::_rewrite('file.php3'));	
		$this->assertEquals('file.shtml.txt', synd_type_file::_rewrite('file.shtml'));	
		$this->assertEquals('file.jsp.txt', synd_type_file::_rewrite('file.jsp'));	
		$this->assertEquals('file.pl.txt', synd_type_file::_rewrite('file.pl'));	
		
		$file = SyndType::factory('file', $this->_path, 'file.php');
		$this->assertEquals('txt', SyndLib::fileExtension($file->path()));
		
		$file->delete();
	}
}
