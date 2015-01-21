<?php
require_once 'PHPUnit2/Framework/TestCase.php';

class _lib_TemplateLib_TestNode {
	var $data = array();
}

class _lib_TemplateLib extends PHPUnit2_Framework_TestCase {
	var $_stack = null;
	var $_requestUri = null;
	
	function setUp() {
		require_once 'core/lib/TemplateLib.inc';
		$this->_stack = isset($_REQUEST['stack']) ? $_REQUEST['stack'] : null;
		$this->_requestUri = $_SERVER['REQUEST_URI'];
	}
	
	function tearDown() {
		if (null !== $this->_stack)
			$_REQUEST['stack'] = $this->_stack;
		$_SERVER['REQUEST_URI'] = $this->_requestUri;
	}
	
	function testView() {
		$actual = tpl_view('node','view','text.123',array('foo' => 'bar', 'foo2' => 'bar2'));
		$expected = tpl_view('node').'view/text.123/?foo=bar&foo2=bar2';
		$this->assertEquals($expected, $actual);

		$actual = tpl_link('node',array('foo' => 'bar', 'foo2' => 'bar2'));
		$expected = tpl_link('node').'?foo=bar&amp;foo2=bar2';
		$this->assertEquals($expected, $actual);
		
		// Test backwards compat
		$actual = tpl_view('node','view','text.123',array('foo' => 'bar', 'foo2' => 'bar2'));
		$expected = tpl_view('node').'view/text.123/?foo=bar&foo2=bar2';
		$this->assertEquals($expected, $actual);

		$actual = tpl_link('node','view','text.123',array('foo' => 'bar', 'foo2' => 'bar2'));
		$expected = tpl_link('node').'view/text.123/?foo=bar&amp;foo2=bar2';
		$this->assertEquals($expected, $actual);
	}
	
	function testViewJump() {
		$actual = tpl_view_jump('node','view','text.123',array('foo' => 'bar', 'foo2' => 'bar2'));
		$expected = tpl_view('node').'view/text.123/?foo=bar&foo2=bar2&stack%5B0%5D='.rawurlencode($_SERVER['REQUEST_URI']);
		$this->assertEquals($expected, $actual);

		$actual = tpl_link_jump('node',array('foo' => 'bar', 'foo2' => 'bar2'));
		$expected = tpl_link('node').'?foo=bar&amp;foo2=bar2&amp;stack%5B0%5D='.rawurlencode($_SERVER['REQUEST_URI']);
		$this->assertEquals($expected, $actual);
		
		// Test backwards compat
		$actual = tpl_view_jump('node','view','text.123',array('foo' => 'bar', 'foo2' => 'bar2'));
		$expected = tpl_view('node').'view/text.123/?foo=bar&foo2=bar2&stack%5B0%5D='.rawurlencode($_SERVER['REQUEST_URI']);
		$this->assertEquals($expected, $actual);

		$actual = tpl_link_jump('node','view','text.123',array('foo' => 'bar', 'foo2' => 'bar2'));
		$expected = tpl_link('node').'view/text.123/?foo=bar&amp;foo2=bar2&amp;stack%5B0%5D='.rawurlencode($_SERVER['REQUEST_URI']);
		$this->assertEquals($expected, $actual);
	}

	function testViewCall() {
		$actual = tpl_view_call('node','view','text.123',array('foo' => 'bar', 'foo2' => 'bar2'));
		$expected = tpl_view('node').'view/text.123/?foo=bar&foo2=bar2&stack%5B0%5D='.rawurlencode($_SERVER['REQUEST_URI']);
		$this->assertEquals($expected, $actual);

		$actual = tpl_link_call('node',array('foo' => 'bar', 'foo2' => 'bar2'));
		$expected = tpl_link('node').'?foo=bar&amp;foo2=bar2&amp;stack%5B0%5D='.rawurlencode($_SERVER['REQUEST_URI']);
		$this->assertEquals($expected, $actual);
		
		// Test backwards compat
		$actual = tpl_view_call('node','view','text.123',array('foo' => 'bar', 'foo2' => 'bar2'));
		$expected = tpl_view('node').'view/text.123/?foo=bar&foo2=bar2&stack%5B0%5D='.rawurlencode($_SERVER['REQUEST_URI']);
		$this->assertEquals($expected, $actual);

		$actual = tpl_link_call('node','view','text.123',array('foo' => 'bar', 'foo2' => 'bar2'));
		$expected = tpl_link('node').'view/text.123/?foo=bar&amp;foo2=bar2&amp;stack%5B0%5D='.rawurlencode($_SERVER['REQUEST_URI']);
		$this->assertEquals($expected, $actual);

		$actual = tpl_view_call('node','view',array('foo' => 'bar', 'foo2' => 'bar2'),'example.php');
		$expected = tpl_view('node').'view/?foo=bar&foo2=bar2&stack%5B0%5D=example.php';
		$this->assertEquals($expected, $actual);

		$actual = tpl_link_call('node','view',array('foo' => 'bar', 'foo2' => 'bar2'),'example.php');
		$expected = tpl_link('node').'view/?foo=bar&amp;foo2=bar2&amp;stack%5B0%5D=example.php';
		$this->assertEquals($expected, $actual);

		// Test encoding
		$_REQUEST['stack'][] = tpl_view('system');
		$_SERVER['REQUEST_URI'] = tpl_view_jump('system');
		$this->assertTrue('%2F' == substr(tpl_view_call('system'),-3));
	}
	
	function testSortUri() {
		$_SESSION['synd']['public']['sort']['_unit_test'] = array('foo',1,'bar',0);
		$uri = tpl_sort_uri('_unit_test', 'foo');
		$this->assertTrue(false !== strpos($uri, '%5Bsort%5D%5B_unit_test%5D%5B0%5D=foo'),1);
		$this->assertTrue(false !== strpos($uri, '%5Bsort%5D%5B_unit_test%5D%5B1%5D=0&'),2);

		$uri = tpl_sort_uri('_unit_test', 'bar');
		$this->assertTrue(false !== strpos($uri, '%5Bsort%5D%5B_unit_test%5D%5B0%5D=bar'),3);
		$this->assertTrue(false !== strpos($uri, '%5Bsort%5D%5B_unit_test%5D%5B1%5D=1&'),4);
	}
	
	function testSortSql() {
		$sql = tpl_sort_sql(array('foo',true,'bar','bar2', false));
		$this->assertEquals('foo, bar, bar2 DESC', $sql);
	}
	
	function testSortList() {
		$a = new _lib_TemplateLib_TestNode();
		$b = new _lib_TemplateLib_TestNode();
		$c = new _lib_TemplateLib_TestNode();
		$d = new _lib_TemplateLib_TestNode();
	
		$a->data['foo'] = 'a123';
		$a->data['bar'] = 'a123';
		
		$b->data['foo'] = 'a123';
		$b->data['bar'] = 'a23';

		$c->data['foo'] = 'c254';
		$c->data['bar'] = 'c254';

		$d->data['foo'] = 'Å223';
		$d->data['bar'] = 'Å23';
		
		$expected = array($c, $b, $a, $d);
		$actual   = array($d, $b, $a, $c);
		tpl_sort_list($actual, array('foo',false, 'bar'));
		
		$this->assertEquals(array_values($expected), array_values($actual));
	}
	
	function testUri() {
		$params = array(
			'foo' => 'bar',
			'a' => array(
				'b' => array('c'=>' '),
				'c' => 2));
				
		$expected = 'foo.php?foo=bar&a%5Bb%5D%5Bc%5D=+&a%5Bc%5D=2';
		$actual = tpl_uri($params, 'foo.php');
		$this->assertEquals($expected, $actual);
	}

	function testHtmlImplode() {
		$this->assertEquals('width="20" height="10"', tpl_html_implode(array('width'=>'20','height'=>10)));
		$this->assertEquals('width:20px; height:10px;', tpl_html_implode(array('width'=>'20px','height'=>'10px'), ':', '; '));
	}
	
	function testVal() {
		$this->assertEquals('&quot;foo&quot;', tpl_value('"foo"'));
		$this->assertEquals('foo', tpl_value(null, 'foo'));
	}
	
	function testChop() {
		$this->assertEquals('foobar', tpl_chop('foobar',6,',.'));
		$this->assertEquals('foob.', tpl_chop('foobar',5,'.'));
		$this->assertEquals('foo..', tpl_chop('foobar',5,'..'));
	}
	
	function testCycle() {
		$this->assertEquals('foo', tpl_cycle(array('foo', 'bar')));
		$this->assertEquals('bar', tpl_cycle());
		$this->assertEquals('foo', tpl_cycle());
	}
	
	function testHtmlLinks() {
		$raw = 'Mer info: www.example.com';
		$expected = 'Mer info: <a target="_new" href="http://www.example.com">www.example.com</a>';
		$actual = tpl_html_links($raw, '_new');
		$this->assertEquals($expected, $actual);

		$raw = 'genom http://www.example.com/ för ';
		$expected = 'genom <a target="_self" href="http://www.example.com/">http://www.example.com/</a> för ';
		$actual = tpl_html_links($raw);
		$this->assertEquals($expected, $actual);

		$raw = 'genom http://www.example.com/index.php?foo=bar&bar=foo, för ';
		$expected = 'genom <a target="_self" href="http://www.example.com/index.php?foo=bar&bar=foo">http://www.example.com/index.php?foo=bar&bar=foo</a>, för ';
		$actual = tpl_html_links($raw);
		$this->assertEquals($expected, $actual);
		
		$raw = "foo
		
http://www.example.com/issue/view/issue.18390/

bar";

		$expected = "foo
		
<a target=\"_self\" href=\"http://www.example.com/issue/view/issue.18390/\">http://www.example.com/issue/view/issue.18390/</a>

bar";
		$actual = tpl_html_links(str_replace("\r", '', $raw));
		$this->assertEquals(str_replace("\r", '', $expected), $actual);

		$raw = 'Some text <a title="Link title http://www.example.com/" href="http://www.example.com/">Link text</a> some http://www.example.com/ other text';
		$expected = 'Some text <a title="Link title http://www.example.com/" href="http://www.example.com/">Link text</a> some <a target="_self" href="http://www.example.com/">http://www.example.com/</a> other text';
		$actual = tpl_html_links($raw);
		$this->assertEquals($expected, $actual);
		
		$raw = 'at &lt;http://example.com/~tarakano/&gt;, it';
		$expected = 'at &lt;<a target="_self" href="http://example.com/~tarakano/">http://example.com/~tarakano/</a>&gt;, it';
		$actual = tpl_html_links($raw);
		$this->assertEquals($expected, $actual);

		$raw = 'http://example.com/~tarakano/, it';
		$expected = '<a target="_self" href="http://example.com/~tarakano/">http://example.com/~tarakano/</a>, it';
		$actual = tpl_html_links($raw);
		$this->assertEquals($expected, $actual);

		$raw = 'http://example.com/~tarakano/. it';
		$expected = '<a target="_self" href="http://example.com/~tarakano/">http://example.com/~tarakano/</a>. it';
		$actual = tpl_html_links($raw);
		$this->assertEquals($expected, $actual);

		$raw = 'ftp.example.com';
		$expected = '<a target="_self" href="ftp://ftp.example.com">ftp.example.com</a>';
		$actual = tpl_html_links($raw);
		$this->assertEquals($expected, $actual);

		$raw = 'foo mikael@example.com bar';
		$expected = 'foo <a href="mailto:mikael@example.com">mikael@example.com</a> bar';
		$actual = tpl_html_links($raw);
		$this->assertEquals($expected, $actual);
	}
	
	function testAttribute() {
		$text = "  123\r\n";
		$expected = "  123  ";
		$this->assertEquals($expected, tpl_attribute($text));
	}
	
	function testMap() {
		$expected = '123';
		$actual = tpl_filter(null, $expected);
		$this->assertEquals($expected, $actual);
		
		$actual = tpl_filter(array($this, '_callback_filter'), '1', '2', '3');
		$this->assertEquals($expected, $actual);
	}
	
	function _callback_filter($a, $b, $c) {
		return "$a$b$c";
	}
	
	function testEmail() {
		$prev = $GLOBALS['synd_user'];

		$GLOBALS['synd_user'] = SyndNodeLib::getInstance('user_case._unit_test');
		$expected = 'To: &quot;<a href="mailto:its-marknad@lists.chalmers.se">its-marknad@lists.chalmers.se</a>&quot; &lt;<a href="mailto:its-marknad@lists.chalmers.se">its-marknad@lists.chalmers.se</a>&gt;';
		$actual = tpl_email('To: "its-marknad@lists.chalmers.se" <its-marknad@lists.chalmers.se>');
		$this->assertEquals($expected, $actual);
		
		$GLOBALS['synd_user'] = SyndNodeLib::getInstance('user_null._unit_test');
		$expected = 'To: &quot;its-marknad AT lists DOT chalmers DOT se&quot; &lt;its-marknad AT lists DOT chalmers DOT se&gt;';
		$actual = tpl_email('To: "its-marknad@lists.chalmers.se" <its-marknad@lists.chalmers.se>');
		$this->assertEquals($expected, $actual);

		$GLOBALS['synd_user'] = $prev;
	}
}
