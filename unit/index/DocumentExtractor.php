<?php
require_once 'unit/index/SyndIndexTestCase.class.inc';
require_once 'core/index/SyndTermIndex.class.inc';
require_once 'core/index/SyndDocumentExtractor.class.inc';

class _index_DocumentExtractor extends SyndIndexTestCase {
	var $_index = null;
	
	function setUp() {
		$search = Module::getInstance('search');
		$this->_index = new SyndTermIndex($search->getIndexDatabase());
	}
	
	function testStrings() {
		$this->_index->loadExtension(new SyndDocumentExtractor());
		
		$file = dirname(__FILE__).'/sample.txt';
		$expected = 
			'Synd Framework '.file_get_contents($file).' '.
			dirname($file).' sample txt';
		$document = $this->_index->fileFragment($file, 'text/plain');
		
		$this->assertNotNull($expected);
		$this->assertEquals($expected, trim($document->toString()));
	}

	function testBuffered() {
		$this->_index->loadExtension(new SyndDocumentExtractor());
		
		$file = md5(uniqid('')).'.txt';
		$expected = 'Some extracted content Some extracted content '.
			dirname($file).' '.SyndLib::chopExtension(basename($file)).' txt';
		$document = $this->_index->fileFragment($file, 'text/plain', 'Some extracted content');
		
		$this->assertNotNull($expected);
		$this->assertEquals($expected, trim($document->toString()));
	}

	function testHTML() {
		$this->_index->loadExtension(new SyndDocumentExtractor());
		
		$file = dirname(__FILE__).'/sample.html';
		$links = array(
			'http://www.synd.info/',
            'http://se.php.net/images/php.gif');
		
		$expected = new SyndDocumentPage(array(
			new SyndDocumentFragment('Synd Framework', 'title', SYND_WEIGHT_TITLE),
			new SyndDocumentFragment(dirname($file), null, SYND_WEIGHT_DIRECTORY),
			new SyndDocumentFragment(SyndLib::chopExtension(basename($file)), null, SYND_WEIGHT_FILENAME),
			new SyndDocumentFragment('Synd is a software framework', 'description', SYND_WEIGHT_DESCRIPTION),
			new SyndDocumentFragment('synd, php, framework', 'subject', SYND_WEIGHT_KEYWORDS),
			new SyndDocumentFragment('Synd homepage', null, SYND_WEIGHT_HEADING),
			new SyndDocumentFragment("  Synd Framework     Synd is a software framework written in  PHP, used primarily for building dynamic websites.  ", null, SYND_WEIGHT_BODY),
			), $links);

		$buffer = file_get_contents($file);
		$buffer = preg_replace('/\s+/', ' ', $buffer);
		
		$actual = $this->_index->fileFragment($file, 'text/html', $buffer);
		$this->assertNotNull($expected);
		$this->assertEquals($expected, $actual);
	}
}
