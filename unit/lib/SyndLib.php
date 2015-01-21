<?php
require_once 'PHPUnit2/Framework/TestCase.php';

class _lib_SyndLib extends PHPUnit2_Framework_TestCase {
	function testArrayKDiff() {
		$a1 = array('1'=>'1', '2'=>'2', '3'=>'3');
		$a2 = array('4'=>'4', '2'=>'5', '6'=>'6');
		$a3 = array('1'=>null, '8'=>'8', '9'=>'9');

		$res = SyndLib::array_kdiff($a1,$a2,$a3);
		$this->assertEquals(2, count($res), 'a1,a2,a3 count(res)');
		$this->assertEquals('1', $res['1'], 'a1,a2,a3 res[1]=>1');
		$this->assertEquals('3', $res['3'], 'a1,a2,a3 res[3]=>3');

		$res = SyndLib::array_kdiff($a3,$a1);
		$this->assertEquals(2, count($res), 'a3,a1 count(res)');
		$this->assertEquals('8', $res['8'], 'a3,a1 res[8]=>8');
		$this->assertEquals('9', $res['9'], 'a3,a1 res[9]=>9');

		$res = SyndLib::array_kdiff($a2,$a1);
		$this->assertEquals(2, count($res), 'a2,a1 count(res)');
		$this->assertEquals('4', $res['4'], 'a2,a1 res[4]=>4');
		$this->assertEquals('6', $res['6'], 'a2,a1 res[6]=>6');
	}

	function testArrayKIntersect() {
		$a1 = array('1'=>'1', '2'=>'2', '3'=>'3');
		$a2 = array('4'=>'4', '2'=>'5', '6'=>'6');
		$a3 = array('1'=>null, '2'=>'8', '9'=>'9');

		$res = SyndLib::array_kintersect($a1,$a2,$a3);
		$this->assertEquals(1, count($res), 'a1,a2,a3 count(res)');
		$this->assertEquals('2', $res['2'], 'a1,a2,a3 res[2]=>2');

		$res = SyndLib::array_kintersect($a3,$a1);
		$this->assertEquals(2, count($res), 'a3,a1 count(res)');
		$this->assertTrue(array_key_exists('1',$res), 'a3,a1 array_key_exists(res[1])');
		$this->assertEquals(null, $res['1'], 'a3,a1 res[1]=>null');
		$this->assertEquals('8', $res['2'], 'a3,a1 res[2]=>8');

		$res = SyndLib::array_kintersect($a3,$a1,true);
		$this->assertEquals(1, count($res), 'a3,a1,stripnulls count(res)');
		$this->assertTrue(!array_key_exists('1',$res), 'a3,a1,stripnulls !array_key_exists(res[1])');
		$this->assertEquals('8', $res['2'], 'a3,a1,stripnulls res[2]=>8');
	}

	function testMap() {
		$this->assertEquals(
			array('foo'=>array('bar'=>'foobar')),
			SyndLib::map('foo','bar','foobar'));
		$this->assertEquals(
			array('foo'=>array('bar'=>array('1'=>'2'))),
			SyndLib::map('foo','bar',array('1'=>'2')));
	}

	function testArrayPrepend() {
		$this->assertEquals(
			array('bar'=>array('bar'=>'foobar'),'foobar'),
			SyndLib::array_prepend(array('bar'=>array('bar'=>'bar'),'bar'), 'foo'));
	}

	function testArrayIntersperse() {
		$this->assertEquals(
			array('bar','foo','bar','foo','bar'),
			SyndLib::array_intersperse(array('bar','bar','bar'),'foo'));
	}

	function testArrayMultisort() {
		$actual = array(
			array('one' => 'c','two' => 'd','three' => 'c'),
			array('one' => 'c','two' => 'e','three' => 'a'),
			array('one' => 'a','two' => 'f','three' => 'a'),
			);
		
		$expected = array(
			array('one' => 'c','two' => 'd','three' => 'c'),
			array('one' => 'c','two' => 'e','three' => 'a'),
			array('one' => 'a','two' => 'f','three' => 'a'),
			);

		$actual = SyndLib::array_multisort($actual, array('one',SORT_DESC,'two'));
		$this->assertEquals(array_values($expected), array_values($actual));

		$expected = array(
			array('one' => 'c','two' => 'd','three' => 'c'),
			array('one' => 'a','two' => 'f','three' => 'a'),
			array('one' => 'c','two' => 'e','three' => 'a'),
			);

		$actual = SyndLib::array_multisort($actual, array('three',SORT_DESC,'two',SORT_DESC));
		$this->assertEquals(array_values($expected), array_values($actual));
	}

	function testFilesTransform() {
		$files = array(
			'file' => array(
				'name' => 'foo.pdf',
				'type' => 'application/pdf'),
			'image' => array(
				'name' => 'foo.gif',
				'type' => 'image/gif'),
			);

		$actual = SyndLib::filesTransform($files);
		$this->assertEquals($files, $actual);

		$files = array(
			'data' => array(
				'name' => array(
					'DATA_FILE' => 'foo.pdf',
					'DATA_IMAGE' => 'foo.jpg'),
				'type' => array(
					'DATA_FILE' => 'application/pdf',
					'DATA_IMAGE' => 'image/gif'),
				)
			);

		$expected = array(
			'data' => array(
				'DATA_FILE' => array(
					'name' => 'foo.pdf',
					'type' => 'application/pdf'),
				'DATA_IMAGE' => array(
					'name' => 'foo.jpg',
					'type' => 'image/gif'),
				)
			);

		$actual = SyndLib::filesTransform($files);
		$this->assertEquals($expected, $actual);

		$files = array(
			'file' => array(
				'name' => 'foo.pdf',
				'type' => 'application/pdf'
				),
			'data' => array(
				'name' => array(
					'DATA_IMAGE' => 'foo.gif'),
				'type' => array(
					'DATA_IMAGE' => 'image/gif'),
				),
			'files' => array(
				'name' => array(
					'data' => array(
						'DATA_IMAGE' => 'foo2.gif'),
					'image' => 'foo3.jpeg',
					),
				'type' => array(
					'data' => array(
						'DATA_IMAGE' => 'image/gif'),
					'image' => 'image/jpeg',
					),
				),
			);

		$expected = array(
			'file' => array(
				'name' => 'foo.pdf',
				'type' => 'application/pdf'
				),
			'data' => array(
				'DATA_IMAGE' => array(
					'name' => 'foo.gif',
					'type' => 'image/gif'),
				),
			'files' => array(
				'data' => array(
					'DATA_IMAGE' => array(
						'name' => 'foo2.gif',
						'type' => 'image/gif'),
					),
				'image' => array(
					'name' => 'foo3.jpeg',
					'type' => 'image/jpeg'),
				),
			);

		$actual = SyndLib::filesTransform($files);
		$this->assertEquals($expected, $actual);
	}

	function testFilePutContents() {
		global $synd_config;
		$buffer = 'bar';
		$path = $synd_config['dirs']['cache'].'_unit_test';
		
		$this->assertFalse(file_exists($path));
		$this->assertTrue(SyndLib::file_put_contents($path, $buffer));
		$this->assertTrue(file_exists($path));
		$this->assertEquals($buffer, file_get_contents($path));

		unlink($path);
	}

	var $_test_hook_flag = false;
	function testHookDynamic() {
		SyndLib::attachHook('_unit_hook_dyn', array($this, '_callback_hook_dynamic'));
		$hres = SyndLib::runHook('_unit_hook_dyn', array('foo' => 'bar'));
		$this->assertEquals('bar', $this->_test_hook_flag, 'failed run: ');
		$this->assertEquals('bar', $hres['foo'], 'failed hres[foo]: ');
	}

	function _callback_hook_dynamic(&$hres, $hparm) {
		$this->assertEquals('bar', $hparm['foo'], '(hook) failed hparm[foo]: ');
		$this->_test_hook_flag = 'bar';
		$hres['foo'] = 'bar';
	}

	function testHookStatic() {
		$GLOBALS['_unit_suite'] = $this;

		SyndLib::attachHook('_unit_hook_stat', array(__CLASS__, '_callback_hook_static'));
		$hres = SyndLib::runHook('_unit_hook_stat', array('foo' => 'bar'));

		global $_test_hook_stat_flag2;
		$this->assertEquals('bar', $GLOBALS['_test_hook_stat_flag1'], 'failed $GLOBALS[flag]: ');
		$this->assertEquals('bar', $_test_hook_stat_flag2, 'failed global $flag2: ');
		$this->assertEquals('bar', $hres['foo'], 'failed hres[foo]: ');
	}

	static function _callback_hook_static(&$hres, $hparm) {
		global $_test_hook_stat_flag2;
		$GLOBALS['_unit_suite']->assertEquals('bar', $hparm['foo'], '(hook) failed hparm[foo]: ');

		$GLOBALS['_test_hook_stat_flag1'] = 'bar';
		$_test_hook_stat_flag2 = 'bar';

		$hres['foo'] = 'bar';
	}

	function testHookRecursive() {
		$this->_test_hook_flag = null;
		SyndLib::attachHook('_unit_hook_rec', array($this, '_callback_test_hook_recursive'));
		$hres = SyndLib::runHook('_unit_hook_rec');
		$this->assertEquals('hook1&hook2', $this->_test_hook_flag);
	}

	function _callback_test_hook_recursive() {
		SyndLib::attachHook('_unit_hook_rec', array($this, '_callback_test_hook_recursive2'));
		$this->_test_hook_flag .= 'hook1';
	}

	function _callback_test_hook_recursive2() {
		$this->_test_hook_flag .= '&hook2';
	}

	function testPreloadedHook() {
		$expected = 'foo';
		$actual = SyndLib::runPreloadedHook('non_existing_hook', $expected);
		$this->assertEquals($expected, $actual);

		SyndLib::attachHook('test_preloaded_hook', array($this, '_callback_test_preloaded_hook'));
		$actual = SyndLib::runPreloadedHook('test_preloaded_hook', $expected);
		$this->assertEquals($expected, $this->_test_hook_flag);
	}
	
	function _callback_test_preloaded_hook(&$result) {
		$this->_test_hook_flag = $result;
	}

	function testFileExtension() {
		$this->assertEquals(null, SyndLib::fileExtension('index'));
		$this->assertEquals('html', SyndLib::fileExtension('index.html'));
		$this->assertEquals('html', SyndLib::fileExtension('foo/bar/index.html'));
		$this->assertEquals('html', SyndLib::fileExtension('/foo/bar/index.html'));
		$this->assertEquals('html', SyndLib::fileExtension('http://www.example.com/foo/bar/index.html'));
	}

	function testChopExtension() {
		$this->assertEquals('index', SyndLib::chopExtension('index'));
		$this->assertEquals('index', SyndLib::chopExtension('index.html'));
		$this->assertEquals('foo/bar/index', SyndLib::chopExtension('foo/bar/index.html'));
		$this->assertEquals('/foo/bar/index', SyndLib::chopExtension('/foo/bar/index.html'));
		$this->assertEquals('http://www.example.com/foo/bar/index', SyndLib::chopExtension('http://www.example.com/foo/bar/index.html'));
	}

	function testInvoke() {
		$node1 = SyndNodeLib::factory('unit_test');
		$node2 = SyndNodeLib::factory('unit_test');
		$nodes = array($node1, $node2->nodeId => $node2);
		
		$expected = array(true, $node2->nodeId => true);
		$actual = SyndLib::invoke($nodes, 'isPermitted', 'get', 'nodeId');
		$this->assertEquals($expected, $actual);

		$expected = array(false, $node2->nodeId => false);
		$actual = SyndLib::invoke($nodes, 'isPermitted', 'get', 'someProperty');
		$this->assertEquals($expected, $actual);
	}
	
	function testSort() {
		$a = new _lib_SyndLibItem('a');
		$b = new _lib_SyndLibItem('b');
		$c = new _lib_SyndLibItem('c');
		
		$expected = array('a' => $a, 'b' => $b, 'c' => $c);
		$actual = SyndLib::sort(array('b' => $b, 'a' => $a, 'c' => $c));
		$this->assertEquals(array_keys($expected), array_keys($actual));

		$expected = array('c' => $c, 'a' => $a, 'b' => $b);
		$actual = SyndLib::sort(array('b' => $b, 'a' => $a, 'c' => $c), 'toString', 'a');
		$this->assertEquals(array_keys($expected), array_keys($actual));
	}
	
	function testFilter() {
		$node1 = SyndNodeLib::getInstance('case._unit_test');
		$node2 = SyndNodeLib::getInstance('null._unit_test');
		$nodes = array($node1, $node2->nodeId => $node2);
		
		$expected = array($node2->nodeId => $node2);
		$actual = SyndLib::filter($nodes, 'isNull');
		$this->assertSame($expected, $actual);
	}
	
	function testSetVar() {
		$expected = uniqid('');
		SyndLib::setVar('_unit_test', $expected);
		$actual = SyndLib::getVar('_unit_test');
		$this->assertEquals($expected, $actual);
	}
	
	function testVariable() {
		$expected = uniqid('');
		SyndLib::runHook('variable_set', 'global', '_unit_test', $expected);
		$actual = SyndLib::runHook('variable_get', 'global', '_unit_test');
		$this->assertEquals($expected, $actual);
	}

	function testDelete() {
		global $synd_maindb;
		$expected = uniqid('');
		SyndLib::runHook('variable_set', 'global', '_unit_test', $expected);
		$actual = SyndLib::runHook('variable_get', 'global', '_unit_test');
		$this->assertEquals($expected, $actual);

		$sql = "
			SELECT 1 FROM synd_variable v
			WHERE v.namespace = 'global' AND v.variable = '_unit_test'";
		$this->assertEquals(1, $synd_maindb->getOne($sql));
		
		SyndLib::runHook('variable_set', 'global', '_unit_test', null);
		
		$sql = "
			SELECT 1 FROM synd_variable v
			WHERE v.namespace = 'global' AND v.variable = '_unit_test'";
		$this->assertNull($synd_maindb->getOne($sql));
	}
	
	function testInstance() {
		$expected = SyndNodeLib::factory('unit_test');
		$actual = SyndLib::getInstance($expected->id());
		$this->assertSame($expected, $actual);
	}

	function testInstanceList() {
		$expected = SyndNodeLib::factory('unit_test');
		$actual = SyndLib::getInstances(array($expected->id()));
		$this->assertSame(array($expected->id() => &$expected), $actual);
	}

	function testMktemp() {
		$tmp = SyndLib::mktemp('unit');
		$this->assertNotNull($tmp);
		$this->assertTrue(SyndLib::file_put_contents($tmp, 'Test'));
		$this->assertTrue(unlink($tmp));
	}
}

class _lib_SyndLibItem {
	private $_string = null;
	
	function __construct($string) {
		$this->_string = $string;
	}
	
	function toString($string = null) {
		return null !== $string ? $string : $this->_string;
	}
}
