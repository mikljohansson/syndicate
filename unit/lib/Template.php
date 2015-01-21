<?php
require_once 'PHPUnit2/Framework/TestCase.php';

class _lib_Template extends PHPUnit2_Framework_TestCase {
	function setUp() {
		require_once 'core/lib/Template.class.inc';
	}
	
	function testDisplay() {
		$expected = 'UnitTest';
		
		$page = new Template();
		$page->assign('var1', 'Unit');
		
		ob_start();
		$page->display(dirname(__FILE__).'/_template/template-001-echo.tpl', array('var2' => 'Test'));
		$actual = ob_get_clean();
		$this->assertEquals($expected, $actual);
		
		$page = new Template(array(dirname(__FILE__).'/'));
		$page->assign('var1', 'Unit');

		ob_start();
		$page->display(dirname(__FILE__).'/_template/template-001-echo.tpl', array('var2' => 'Test'));
		$actual = ob_get_clean();
		$this->assertEquals($expected, $actual);

		ob_start();
		$page->display('_template/template-001-echo.tpl', array('var2' => 'Test'));
		$actual = ob_get_clean();
		$this->assertEquals($expected, $actual);
	}
	
	function testFetch() {
		$expected = 'UnitTest';
		
		$page = new Template();
		$page->assign('var1', 'Unit');
		
		$actual = $page->fetch(dirname(__FILE__).'/_template/template-001-echo.tpl', array('var2' => 'Test'));
		$this->assertEquals($expected, $actual);
		
		$page = new Template(array(dirname(__FILE__).'/'));
		$page->assign('var1', 'Unit');

		$actual = $page->fetch(dirname(__FILE__).'/_template/template-001-echo.tpl', array('var2' => 'Test'));
		$this->assertEquals($expected, $actual);

		$actual = $page->fetch('_template/template-001-echo.tpl', array('var2' => 'Test'));
		$this->assertEquals($expected, $actual);
	}

	function testDisplayOnce() {
		$expected = 'UnitTest';
		
		$page = new Template();
		$page->assign('var1', 'Unit');
		
		ob_start();
		$page->displayonce(dirname(__FILE__).'/_template/template-001-echo.tpl', array('var2' => 'Test'));
		$actual = ob_get_clean();
		$this->assertEquals($expected, $actual);
		
		ob_start();
		$page->displayonce(dirname(__FILE__).'/_template/template-001-echo.tpl', array('var2' => 'Test'));
		$actual = ob_get_clean();
		$this->assertEquals('', $actual);
	}
	
	function testRender() {
		$expected = 'UnitTestObject';

		$page = new Template(array(dirname(__FILE__).'/_template/'));
		$page->assign('var1', 'Unit');

		ob_start();
		$page->render(new synd_template_test(), 'template-001-echo.tpl', array('var2' => 'Test'));
		$actual = ob_get_clean();
		$this->assertEquals($expected, $actual);

		ob_start();
		$expected = 'UnitTestTest2';
		$node1 = new synd_template_test('Test1');
		$node2 = new synd_template_test('Test2');
		$page->render($node1, 'template-001-echo.tpl', array('var2' => 'Test', 'node' => $node2));
		$actual = ob_get_clean();
		$this->assertEquals($expected, $actual);
	}

	function testFetchNode() {
		$expected = 'UnitTestObject';

		$page = new Template(array(dirname(__FILE__).'/_template/'));
		$page->assign('var1', 'Unit');

		$actual = $page->fetchnode(new synd_template_test(), 'template-001-echo.tpl', array('var2' => 'Test'));
		$this->assertEquals($expected, $actual);
	}

	function testIterate() {
		$expected = 'UnitTestObject1UnitTestObject2';

		$page = new Template(array(dirname(__FILE__).'/_template/'));
		$page->assign('var1', 'Unit');

		ob_start();
		$page->iterate(array(new synd_template_test('Object1'), new synd_template_test('Object2')), 'template-001-echo.tpl', array('var2' => 'Test'));
		$actual = ob_get_clean();
		$this->assertEquals($expected, $actual);
	}
	
	function testFetchIterator() {
		$expected = 'UnitTestObject1UnitTestObject2';

		$page = new Template(array(dirname(__FILE__).'/_template/'));
		$page->assign('var1', 'Unit');

		$actual = $page->fetchiterator(array(new synd_template_test('Object1'), new synd_template_test('Object2')), 'template-001-echo.tpl', array('var2' => 'Test'));
		$this->assertEquals($expected, $actual);
	}

	function testQuote() {
		$page = new Template();
		$expected = '&lt;';
		$actual = $page->quote('<');
		$this->assertEquals($expected, $actual);
	}

	function testText() {
		$page = new Template();
		$expected = '__unit_test abc 123';
		$actual = $page->text('__unit_test %s %d', 'abc', '123');
		$this->assertEquals($expected, $actual);
	}
	
	function testCycle() {
		$page = new Template();
		$this->assertEquals(1, $page->cycle(array(1,2,3)));
		$this->assertEquals(2, $page->cycle());
		$this->assertEquals(3, $page->cycle());
		$this->assertEquals(1, $page->cycle());
	}
	
	function testSequence() {
		$page = new Template();
		$this->assertEquals(1, $page->sequence());
		$this->assertEquals(0, $page->sequence(0));
		$this->assertEquals(1, $page->sequence());
		$this->assertEquals(2, $page->sequence());
		$this->assertEquals(1, $page->sequence(1));
		$this->assertEquals(2, $page->sequence());
	}
	
	function testLinks() {
		$page = new Template();
		
		$raw = 'Mer info: www.example.com';
		$expected = 'Mer info: <a href="http://www.example.com">www.example.com</a>';
		$actual = $page->links($raw);
		$this->assertEquals($expected, $actual);

		$raw = 'genom http://www.example.com/ för ';
		$expected = 'genom <a href="http://www.example.com/">http://www.example.com/</a> för ';
		$actual = $page->links($raw);
		$this->assertEquals($expected, $actual);

		$raw = 'genom http://www.example.com/index.php?foo=bar&bar=foo, för ';
		$expected = 'genom <a href="http://www.example.com/index.php?foo=bar&bar=foo">http://www.example.com/index.php?foo=bar&bar=foo</a>, för ';
		$actual = $page->links($raw);
		$this->assertEquals($expected, $actual);
		
		$raw = "foo
		
http://www.example.com/issue/view/issue.18390/

bar";

		$expected = "foo
		
<a href=\"http://www.example.com/issue/view/issue.18390/\">http://www.example.com/issue/view/issue.18390/</a>

bar";
		$actual = $page->links(str_replace("\r", '', $raw));
		$this->assertEquals(str_replace("\r", '', $expected), $actual);

		$raw = 'Some text <a title="Link title http://www.example.com/" href="http://www.example.com/">Link text</a> some http://www.example.com/ other text';
		$expected = 'Some text <a title="Link title http://www.example.com/" href="http://www.example.com/">Link text</a> some <a href="http://www.example.com/">http://www.example.com/</a> other text';
		$actual = $page->links($raw);
		$this->assertEquals($expected, $actual);
		
		$raw = 'at &lt;http://example.com/~tarakano/&gt;, it';
		$expected = 'at &lt;<a href="http://example.com/~tarakano/">http://example.com/~tarakano/</a>&gt;, it';
		$actual = $page->links($raw);
		$this->assertEquals($expected, $actual);

		$raw = 'http://example.com/~tarakano/, it';
		$expected = '<a href="http://example.com/~tarakano/">http://example.com/~tarakano/</a>, it';
		$actual = $page->links($raw);
		$this->assertEquals($expected, $actual);

		$raw = 'http://example.com/~tarakano/. it';
		$expected = '<a href="http://example.com/~tarakano/">http://example.com/~tarakano/</a>. it';
		$actual = $page->links($raw);
		$this->assertEquals($expected, $actual);

		$raw = 'ftp.example.com';
		$expected = '<a href="ftp://ftp.example.com">ftp.example.com</a>';
		$actual = $page->links($raw);
		$this->assertEquals($expected, $actual);

		$raw = 'foo mikael@example.com bar';
		$expected = 'foo <a href="mailto:mikael@example.com">mikael@example.com</a> bar';
		$actual = $page->links($raw);
		$this->assertEquals($expected, $actual);
	}
	
	function testFormat() {
		$page = new Template();
		
		$raw = "  Test\t\r\nTest";
		$expected = "&nbsp;&nbsp;Test&nbsp;&nbsp;&nbsp;&nbsp;<br />\r\nTest";
		$actual = $page->format($raw);
		$this->assertEquals($expected, $actual);
	}
	
	function testUri() {
		$page = new Template(null, new HttpRequest($_SESSION, '/synd/'));
		$expected = '/synd/project/test/?getvar=Test+Variable&offset=100';
		$actual = $page->uri('project', 'test', array('getvar' => 'Test Variable', 'offset' => 100));
		$this->assertEquals($expected, $actual);
	}
	
	function testHref() {
		$page = new Template(null, new HttpRequest($_SESSION, '/synd/'));
		$expected = '/synd/project/test/?getvar=Test+Variable&amp;offset=100';
		$actual = $page->href('project', 'test', array('getvar' => 'Test Variable', 'offset' => 100));
		$this->assertEquals($expected, $actual);
	}
	
	function testCall() {
		$page = new Template(null, new HttpRequest($_SESSION, '/synd/', 'issue/12345/', array('test'=>'123'), array('test'=>'123')));
		$expected = '/synd/project/test/?getvar=Test+Variable&amp;offset=100&amp;stack%5B0%5D=%2Fsynd%2Fissue%2F12345%2F%3Ftest%3D123';
		$actual = $page->call('project', 'test', array('getvar' => 'Test Variable', 'offset' => 100));
		$this->assertEquals($expected, $actual);
	}

	function testJump() {
		$get = array('foo'=>'bar','stack'=>array('/synd/issue/123/'));
		$page = new Template(null, new HttpRequest($_SESSION, '/synd/', 'project/example/issues/', $get, $get));
		$expected = '/synd/project/test/?getvar=Test+Variable&amp;offset=100&amp;stack%5B0%5D=%2Fsynd%2Fissue%2F123%2F';
		$actual = $page->jump('project', 'test', array('getvar' => 'Test Variable', 'offset' => 100));
		$this->assertEquals($expected, $actual);
	}

	function testMerge() {
		$get = array('getvar'=>'Test','offset'=>50);
		$post = array('postvar' => 'Test2');
		$request = new HttpRequest($_SESSION, '/synd/', 'issue/project/test/', array_merge($get, $post), $get, $post, array(), array());
		
		$page = new Template(null, $request);
		$expected = $page->href('issue','project','test').'?getvar=Test&offset=100';
		$actual = $page->merge(array('offset' => 100));
		
		$this->assertEquals($expected, $actual);
	}
}

class synd_template_test {
	public $_var = null;
	
	function __construct($var = 'Object') {
		$this->_var = $var;
	}
}
