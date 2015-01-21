<?php
require_once 'PHPUnit2/Framework/TestCase.php';

class _lib_AsyncHttpConnection extends PHPUnit2_Framework_TestCase {
	var $_path = null;
	
	function setUp() {
		require_once 'core/lib/AsyncHttpConnection.class.inc';
		$this->_path = substr(dirname(__FILE__), strlen(rtrim($_SERVER['DOCUMENT_ROOT'],DIRECTORY_SEPARATOR)));
	}
	
	function testRequest() {
		$callback = new _UnittestHTTPAsyncCallback();
		$connection = new AsyncHttpConnection("http://{$_SERVER['SERVER_NAME']}", $callback);
		$connection->getRequest("{$this->_path}/_http/httpconnection-001-head.txt");

		// More than 1 send + 1 recv packets should not be needed
		$ts = time();
		while ($ts + 2 > time() && null == $callback->_response) {
			usleep(100);
			AsyncHttpConnection::select();
		}

		$this->assertNotNull($callback->_connection);
		
		$headers = $connection->getHeaders();
		$this->assertEquals('200', $connection->getStatus());
		$this->assertEquals('4', $headers['content-length']);
		$this->assertEquals('Test', $callback->_response);
		$type = explode(';', $headers['content-type']);
		$this->assertEquals('text/plain', $type[0]);
	}

	function testHeadRequest() {
		$callback = new _UnittestHTTPAsyncCallback();
		$connection = new AsyncHttpConnection("http://{$_SERVER['SERVER_NAME']}", $callback);
		$connection->headRequest("{$this->_path}/_http/httpconnection-001-head.txt");

		$callback2 = new _UnittestHTTPAsyncCallback();
		$connection2 = new AsyncHttpConnection("http://{$_SERVER['SERVER_NAME']}", $callback2);
		$connection2->headRequest("{$this->_path}/_http/httpconnection-001-head-nonexistant.txt");

		// More than 1 send + 1 recv packets should not be needed
		$ts = time();
		while ($ts + 5 > time() && (null === $callback->_response || null === $callback2->_response)) {
			usleep(200);
			AsyncHttpConnection::select();
		}

		$headers = $connection->getHeaders();
		$this->assertEquals('200', $connection->getStatus());
		$this->assertEquals('4', $headers['content-length']);
		$type = explode(';', $headers['content-type']);
		$this->assertEquals('text/plain', $type[0]);

		$headers2 = $connection2->getHeaders();
		$this->assertEquals('404', $connection2->getStatus());
	}

	function testMultipart() {
		$callback = new _UnittestHTTPAsyncCallback();
		$connection = new AsyncHttpConnection("http://{$_SERVER['SERVER_NAME']}", $callback);
		$connection->postMultipart("{$this->_path}/_http/httpconnection-002-multipart.script.php",
			array('field' => '123'), array('file1' => 'content1', 'file2' => array('file2.txt', 'content2')));

		// More than 1 send + 1 recv packets should not be needed
		$ts = time();
		while ($ts + 2 > time() && null == $callback->_response) {
			usleep(100);
			AsyncHttpConnection::select();
		}

		$expected = "field 123\nfile1 file1 content1\nfile2 file2.txt content2\n";
		$this->assertEquals($expected, $callback->_response);
	}
	
	function testRedirect() {
		$callback = new _UnittestHTTPAsyncCallback();
		$connection = new AsyncHttpConnection("http://{$_SERVER['SERVER_NAME']}", $callback);
		$connection->getRequest("{$this->_path}/_http/httpconnection-003-redirect.script.php");

		// More than (1 send + 1 recv) * 2 packets should not be needed
		$ts = time();
		while ($ts + 2 > time() && null == $callback->_response) {
			AsyncHttpConnection::select();
			usleep(100);
		}
		
		$this->assertNotNull($callback->_connection);
		
		$headers = $connection->getHeaders();
		$this->assertEquals('200', $connection->getStatus());
		$this->assertEquals('4', $headers['content-length']);
		$this->assertEquals('Test', $callback->_response);
		$type = explode(';', $headers['content-type']);
		$this->assertEquals('text/plain', $type[0]);
	}

	function testConnectionClose() {
		$callback = new _UnittestHTTPAsyncCallback();
		$connection = new AsyncHttpConnection("http://{$_SERVER['SERVER_NAME']}", $callback);
		$connection->getRequest("{$this->_path}/_http/httpconnection-004-close.script.php");

		// More than 1 send + 1 recv packets should not be needed
		$ts = time();
		while ($ts + 2 > time() && null == $callback->_response) {
			usleep(100);
			AsyncHttpConnection::select();
		}

		$this->assertNotNull($callback->_connection);
		
		$headers = $connection->getHeaders();
		$this->assertEquals('200', $connection->getStatus());
		$this->assertEquals('Test', $callback->_response);
		$this->assertEquals('close', $headers['connection']);
		$type = explode(';', $headers['content-type']);
		$this->assertEquals('text/html', $type[0]);
	}

	function testChunkedResponse() {
		$callback = new _UnittestHTTPAsyncCallback();
		$connection = new AsyncHttpConnection("http://{$_SERVER['SERVER_NAME']}", $callback);
		$connection->getRequest("{$this->_path}/_http/httpconnection-005-chunked.script.php");

		// More than 1 send + 1 recv packets should not be needed
		$ts = time();
		while ($ts + 2 > time() && null == $callback->_response) {
			usleep(100);
			AsyncHttpConnection::select();
		}

		$this->assertNotNull($callback->_connection);

		$headers = $connection->getHeaders();
		$this->assertEquals('200', $connection->getStatus());
		$this->assertEquals('Test', $callback->_response);
		$this->assertEquals('ascii, chunked', $headers['transfer-encoding']);
		$type = explode(';', $headers['content-type']);
		$this->assertEquals('text/html', $type[0]);

		$this->assertEquals('Test', $headers['some-header']);
		$this->assertEquals('Test2', $headers['some-header2']);
	}
}

class _UnittestHTTPAsyncCallback {
	var $_connection = null;
	var $_response = null;
	
	function httpResponse($connection, $response) {
		$this->_connection = $connection;
		$this->_response = null == $response ? true : $response;
	}
	
	function httpError($connection) {
		$this->_response = '';
	}
}