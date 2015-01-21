<?php
require_once 'unit/index/SyndIndexTestCase.class.inc';
require_once 'core/index/SyndNullIndex.class.inc';
require_once 'core/index/SyndTextFilter.class.inc';
require_once 'core/index/SyndDocument.class.inc';

class _index_Document extends SyndIndexTestCase {
	function testFragment() {
		$index = new SyndNullIndex();
		$extension = new SyndTextFilter();
		$index->loadExtension($extension);

		$str = 'Simple document fragment test.';
		$document = new SyndDocumentFragment($str);
		$this->assertEquals($str, $document->toString());
		
		$str2 = 'Simple document fragment test.';
		$document->acceptVisitor($index);
		$this->assertEquals($str2, $document->toString());
	}

	function testComposite() {
		$str = 'Simple document';
		$str2 = 'composite test.';
		
		$document = new SyndDocumentComposite(array(
			new SyndDocumentFragment($str),
			new SyndDocumentFragment($str2)));
		
		$this->assertEquals("$str $str2", $document->toString());
	}
}

?>