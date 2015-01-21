<?php
require_once 'PHPUnit2/Framework/TestCase.php';

class _vendor_Xapian extends PHPUnit2_Framework_TestCase {
	var $_path = null;
	var $_index = null;

	function setUp() {
		global $synd_config;
		require_once 'core/index/XapianIndex.class.inc';
		require_once 'core/index/SyndTextFilter.class.inc';
		require_once 'core/index/SyndStemFilter.class.inc';
		require_once 'core/index/SyndFieldExtension.class.inc';
		require_once 'core/index/IndexQuery.class.inc';

		$this->_path = '/tmp/xapian-unit_test-'.md5(uniqid(''));
		SyndLib::createDirectory($this->_path);
		
		$this->_index = new XapianIndex($this->_path);
		$this->_index->loadExtension(new SyndTextFilter());
		$this->_index->loadExtension(new SyndStemFilter('en'));
	}
	
	function tearDown() {
		$this->_index->flush();
		SyndLib::unlink($this->_path, true);
	}
	
	function testSearch() {
		$this->_index->clearSection('_unit_test');
		$id = '43cff030cecc6afedklakjkghcb587ac';

		$this->_index->addDocument('_unit_test1',
			$this->_index->createFragment("$id foo"),
			'_unit_test');
		$this->_index->addDocument('_unit_test2',
			$this->_index->createFragment("$id bar"),
			'_unit_test');
		$this->_index->addDocument('_unit_test3',
			$this->_index->createFragment("123a4b5"),
			'_unit_test');
		$this->_index->addDocument('_unit_test4',
			$this->_index->createFragment("123"),
			'_unit_test');
		$this->_index->addDocument('_unit_test5',
			$this->_index->createFragment("a2abc"),
			'_unit_test');
			
		$this->_index->flush();
		
		$mset = $this->_index->getMatchSet(new IndexQuery("$id", array('_unit_test')), 0, 10);
		$mset = SyndLib::array_collect(array_values($mset), 0);
		sort($mset);
		$this->assertEquals(array('_unit_test1','_unit_test2'), $mset);

		$mset = $this->_index->getMatchSet(new IndexQuery("+$id -bar", array('_unit_test')), 0, 10);
		$mset = SyndLib::array_collect(array_values($mset), 0);
		$this->assertEquals(array('_unit_test1'), $mset);

		$mset = $this->_index->getMatchSet(new IndexQuery("123", array('_unit_test')), 0, 10);
		$mset = SyndLib::array_collect(array_values($mset), 0);
		sort($mset);
		$this->assertEquals(array('_unit_test4'), $mset);
		
		$mset = $this->_index->getMatchSet(new IndexQuery("123a4b5", array('_unit_test')), 0, 10);
		$mset = SyndLib::array_collect(array_values($mset), 0);
		$this->assertEquals(array('_unit_test3'), $mset);
		
		$mset = $this->_index->getMatchSet(new IndexQuery("a2abc", array('_unit_test')), 0, 10);
		$mset = SyndLib::array_collect(array_values($mset), 0);
		$this->assertEquals(array('_unit_test5'), $mset);

		// Test delete_document
		$mset = $this->_index->getMatchSet(new IndexQuery("$id", array('_unit_test')), 0, 10);
		$mset = SyndLib::array_collect($mset, 0);
		$this->assertEquals(2, count($mset));
		
		// Delete by docid
		$this->_index->delDocument('_unit_test1');
		$mset = $this->_index->getMatchSet(new IndexQuery("$id", array('_unit_test')), 0, 10);
		$this->assertEquals(1, count($mset));
		
		// Delete by unique term
		$this->_index->delDocument('_unit_test2');
		$mset = $this->_index->getMatchSet(new IndexQuery("$id", array('_unit_test')), 0, 10);
		$this->assertEquals(0, count($mset));
	}
	
//	function testFieldSearch() {
//		$this->_index->loadExtension(new SyndFieldExtension());
//
//		$this->_index->addDocument('_unit_test1',
//			$this->_index->createComposite(array(
//				$this->_index->createFragment('abcdef', 'title'),
//				$this->_index->createFragment('foo'),
//			)),
//			'_unit_test');
//
//		$this->_index->addDocument('_unit_test2',
//			$this->_index->createComposite(array(
//				$this->_index->createFragment('abcdef'),
//				$this->_index->createFragment('foo'),
//				)),
//			'_unit_test');
//			
//		$this->_index->flush();
//		
//		$mset = $this->_index->getMatchSet(new IndexQuery('abcdef', array('_unit_test')), 0, 10);
//		$mset = SyndLib::array_collect($mset, 0);
//		sort($mset);
//		$this->assertEquals(array('_unit_test1','_unit_test2'), $mset);
//
//		$mset = $this->_index->getMatchSet(new IndexQuery('title:abcdef', array('_unit_test')), 0, 10);
//		$mset = SyndLib::array_collect($mset, 0);
//		$this->assertEquals(array('_unit_test1'), $mset);
//	}
}