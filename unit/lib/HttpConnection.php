<?php
require_once 'PHPUnit2/Framework/TestCase.php';

class _lib_HttpConnection extends PHPUnit2_Framework_TestCase {
	var $_path = null;
	
	function setUp() {
		require_once 'core/lib/HttpConnection.class.inc';
		$this->_path = substr(dirname(__FILE__), strlen(rtrim($_SERVER['DOCUMENT_ROOT'],DIRECTORY_SEPARATOR)));
	}
	
	function testRequest() {
		$connection = new HttpConnection(tpl_request_host());
		$response = $connection->getRequest("{$this->_path}/_http/httpconnection-001-head.txt");
		$headers = $connection->getHeaders();
		
		$this->assertEquals('200', $connection->getStatus());
		$this->assertEquals('4', $headers['content-length']);
		$this->assertEquals('Test', $response);

		$type = explode(';', $headers['content-type']);
		$this->assertEquals('text/plain', $type[0]);
	}
	
	function testPostRequest() {
		$expected = str_repeat(md5(uniqid('')), 1024*8);
		$connection = new HttpConnection(tpl_request_host());
		$response = $connection->postRequest("{$this->_path}/_http/httpconnection-007-post.script.php", $expected, 'application/octet-stream');
		$this->assertEquals($expected, $response);
		$this->assertEquals(strlen($expected), strlen($response));
	}
	
	function testHeadRequest() {
		$connection = new HttpConnection(tpl_request_host());
		$headers = $connection->headRequest("{$this->_path}/_http/httpconnection-001-head.txt");
		$this->assertEquals('200', $connection->getStatus());
		$this->assertEquals('4', $headers['content-length']);

		$type = explode(';', $headers['content-type']);
		$this->assertEquals('text/plain', $type[0]);

		$connection = new HttpConnection(tpl_request_host());
		$headers = $connection->headRequest("{$this->_path}/_http/httpconnection-001-head-nonexistant.txt");
		$this->assertEquals('404', $connection->getStatus());

		$type = explode(';', $headers['content-type']);
		$this->assertEquals('text/html', $type[0]);
	}
	
	function testMultipart() {
		$connection = new HttpConnection(tpl_request_host());
		$response = $connection->postMultipart("{$this->_path}/_http/httpconnection-002-multipart.script.php",
			array('field' => '123'), array('file1' => 'content1', 'file2' => array('file2.txt', 'content2')));
		$expected = "field 123\nfile1 file1 content1\nfile2 file2.txt content2\n";
		$this->assertEquals($expected, $response);
	}
	
	function testRedirect() {
		$connection = new HttpConnection(tpl_request_host());
		$response = $connection->getRequest("{$this->_path}/_http/httpconnection-003-redirect.script.php");
		$headers = $connection->getHeaders();
		
		$this->assertEquals('200', $connection->getStatus());
		$this->assertEquals('4', $headers['content-length']);
		$this->assertEquals('Test', $response);

		$type = explode(';', $headers['content-type']);
		$this->assertEquals('text/plain', $type[0]);

		$connection = new HttpConnection(tpl_request_host());
		$response = $connection->getRequest("{$this->_path}/_http/httpconnection-008-redirect-absolute.script.php");
		$headers = $connection->getHeaders();
		
		$this->assertEquals('200', $connection->getStatus());
		$this->assertEquals('4', $headers['content-length']);
		$this->assertEquals('Test', $response);

		$type = explode(';', $headers['content-type']);
		$this->assertEquals('text/plain', $type[0]);
	}
	
	function testConnectionClose() {
		$connection = new HttpConnection(tpl_request_host());
		$response = $connection->getRequest("{$this->_path}/_http/httpconnection-004-close.script.php");
		$headers = $connection->getHeaders();
		
		$this->assertEquals('200', $connection->getStatus());
		$this->assertEquals('Test', $response);
		$this->assertEquals('close', $headers['connection']);

		$type = explode(';', $headers['content-type']);
		$this->assertEquals('text/html', $type[0]);
	}

	function testChunkedResponse() {
		$connection = new HttpConnection(tpl_request_host());
		$response = $connection->getRequest("{$this->_path}/_http/httpconnection-005-chunked.script.php");
		$headers = $connection->getHeaders();
		
		$this->assertEquals('200', $connection->getStatus());
		$this->assertEquals('Test', $response);
		$this->assertEquals('ascii, chunked', $headers['transfer-encoding']);

		$this->assertEquals('Test', $headers['some-header']);
		$this->assertEquals('Test2', $headers['some-header2']);

		$type = explode(';', $headers['content-type']);
		$this->assertEquals('text/html', $type[0]);
	}
	
	function testRelative() {
		$connection = new HttpConnection('http://www.example.com');
		
		$this->assertEquals('http://www.example.com/index.html', 
			$connection->relative('/', '//www.example.com/index.html'));

		$this->assertEquals('http://www.example.com/section/index.html', 
			$connection->relative('/section/web/apache.html', '../index.html'));
		$this->assertEquals('http://www.example.com/index.html', 
			$connection->relative('/section/web/apache.html', '../../index.html'));
		$this->assertEquals('http://www.example.com/index.html', 
			$connection->relative('/section/apache.html', '../../index.html'));

		$this->assertEquals('http://www.example.com/index.html', 
			$connection->relative('/apache.html', 'index.html'));
		$this->assertEquals('http://www.example.com/apache/index.html', 
			$connection->relative('/apache/index.html', 'index.html'));
		$this->assertEquals('http://www.example.com/apache/index.html', 
			$connection->relative('/apache/', 'index.html'));
		$this->assertEquals('http://www.example.com/index.html', 
			$connection->relative('/apache/', '/index.html'));

		$this->assertEquals('http://www.example.com/index.html', 
			$connection->relative('/section/apache.html', '/index.html'));
	}
}