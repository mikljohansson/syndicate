<?php
require_once 'PHPUnit2/Framework/TestCase.php';

class _modules_Controller extends PHPUnit2_Framework_TestCase {
	function setUp() {
		require_once 'core/Controller.class.inc';
	}
	
	function testRequest() {
		$context	= 'issue/12345/';
		$get		= array('getvar' => 'Test');
		$post		= array('postvar' => 'Test2');
		$req		= array_merge($get, $post);
		$files		= array('data' => array('file' => array('tmp_name' => '/tmp/foo', 'name' => 'Foo')));
		$session	= array('sessvar' => 'Test3');
		$cookie		= array('cookievar' => 'Test5');
		
		$request = new HttpRequest($session, '/synd/', $context, $req, $get, $post, $files, $cookie);
		
		$this->assertEquals('issue', $request[0]);
		$this->assertEquals('12345', $request[1]);
		
		$request2 = $request->forward();
		$this->assertEquals('12345', $request[0]);
		$this->assertEquals(explode('/', trim($context, '/')), $request->context);
		
		$this->assertEquals('Test', $request['getvar']);
		$this->assertEquals('Test', $request->get['getvar']);
		$this->assertEquals('Test2', $request['postvar']);
		$this->assertEquals('Test2', $request->post['postvar']);
		
		$this->assertEquals('/tmp/foo', $request['data']['file']['tmp_name']);
		
		$this->assertEquals('Test3', $request->session['sessvar']);
		$this->assertFalse(isset($request->session['sessvar2']));
		$request->session['sessvar2'] = 'Test4';
		$this->assertTrue(isset($request->session['sessvar2']));
		$this->assertEquals('Test4', $request->session['sessvar2']);
		
		$this->assertEquals('Test5', $request->cookie['cookievar']);
	}
}
