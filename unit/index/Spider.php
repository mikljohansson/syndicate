<?php
require_once 'unit/index/SyndIndexTestCase.class.inc';
require_once 'core/index/SyndSpider.class.inc';
require_once 'core/index/SyndWebIndex.class.inc';
require_once 'core/index/SyndNullIndex.class.inc';

class _index_Spider extends SyndIndexTestCase {
	function setUp() {
		$this->_path = substr(dirname(__FILE__), strlen(rtrim($_SERVER['DOCUMENT_ROOT'],DIRECTORY_SEPARATOR)));
	}
	
	function testSpider() {
		$backend = new SyndNullIndex();
		$backend->loadExtension(new SyndDocumentExtractor());
		$index = new _UnittestWebIndex($backend);

		$search = Module::getInstance('search');
		$queue = new SyndBufferedSpiderQueue(new _UnittestWebSpiderQueue());
		$queue->addLocation("http://{$_SERVER['SERVER_NAME']}{$this->_path}/spider-001.html");
		
		$spider = new SyndSpider($queue, $index, 0);
		$spider->run();
		
		$uris = array_keys($index->_documents);
		sort($uris);

		$this->assertEquals(array(
			"http://{$_SERVER['SERVER_NAME']}{$this->_path}/spider-001.html",
			"http://{$_SERVER['SERVER_NAME']}{$this->_path}/spider-002.html",
			"http://{$_SERVER['SERVER_NAME']}{$this->_path}/spider-003.txt",
			), $uris);
	}
	
	function testFilter() {
		$backend = new SyndNullIndex();
		$backend->loadExtension(new SyndDocumentExtractor());
		$index = new _UnittestWebIndex($backend);

		$search = Module::getInstance('search');
		$queue = new SyndBufferedSpiderQueue(new _UnittestWebSpiderQueue());
		$queue->addLocation("http://{$_SERVER['SERVER_NAME']}{$this->_path}/spider-001.html");
		
		$spider = new SyndSpider($queue, $index, 0);
		$spider->loadExtension(new SyndSpiderURIFilter(
			array("/^https?:\/\/\w*{$_SERVER['SERVER_NAME']}(:\d+)?($|\/.*)/i"),
			array('/\.html$/i'),
			array(),
			array('/^text\/(html|plain)\b/i',)
			));
		$spider->run();
		
		$uris = array_keys($index->_documents);
		sort($uris);
		
		$this->assertEquals(array(
			"http://{$_SERVER['SERVER_NAME']}{$this->_path}/spider-001.html",
			"http://{$_SERVER['SERVER_NAME']}{$this->_path}/spider-002.html",
			), $uris);
	}
}

class _UnittestWebIndex extends AbstractIndex {
	var $_index = null;
	var $_documents = array();
	
	function _UnittestWebIndex($backend) {
		$this->_index = $backend;
	}
	
	function attachHook($name, $callback, $priority = 0) {
		$this->_index->attachHook($name, $callback, $priority);
	}
	
	function getDocumentStructure($type, $uri, $response, &$title, &$content) {
		return $this->_index->runHook('extract', 
			array($type, $uri, $response, &$title, &$content));
	}
	
	function addDocument($uri, $title, $content, $document, $size, $modified = 0) {
		$this->_documents[$uri] = $document;
		$this->_index->addDocument($uri, $document);
	}
	
	function delDocument($uri) {
		unset($this->_documents[$uri]);
	}
	
	function isBuffered($docid) {
		return false !== array_search($docid, array_map('crc32', array_keys($this->_documents)));
	}
}

class _UnittestWebSpiderQueue {
	var $_locations = array();
	
	function getLocations($limit) {
		return array_splice($this->_locations, 0, $limit);
	}
	
	function addLocations($uris) {
		$this->_locations = array_merge($this->_locations, $uris);
	}
}