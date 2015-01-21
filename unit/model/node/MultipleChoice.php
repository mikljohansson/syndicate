<?php
require_once 'PHPUnit2/Framework/TestCase.php';

class _model_Node_MultipleChoice extends PHPUnit2_Framework_TestCase {
	function testExtractInlineOptions() {
		$node = SyndNodeLib::factory('multiple_choice');
		$data = array(
			'INFO_LAYOUT' => 'inline_radio',
			'INFO_QUESTION' => 'Sample <(2) question> with <(1) options> <(3)> inline');
		
		$this->assertEquals(array(), $node->getOptions());
		$node->merge($data);

		$this->assertEquals($data['INFO_LAYOUT'], $node->getLayout());
		$this->assertEquals($data['INFO_QUESTION'], $node->toString());
		
		$expected = array('2','1','3');
		$actual = SyndLib::array_collect($node->getOptions(), 'INFO_OPTION');
		$this->assertEquals($expected, $actual);
	
		$this->assertFalse(empty($node->_options));
		$node->save();
		$node->flush();
		$this->assertTrue(empty($node->_options));
		
		$actual = SyndLib::array_collect($node->getOptions(), 'INFO_OPTION');
		$this->assertEquals($expected, $actual);

		$node->delete();
	}
	
	function testClone() {
		$node = SyndNodeLib::factory('multiple_choice');
		$data = array(
			'INFO_LAYOUT' => 'inline_radio',
			'INFO_QUESTION' => 'Sample <(2) question> with <(1) options> <(3)> inline');
		
		$this->assertEquals(array(), $node->getOptions());
		$node->merge($data);

		$expected = array('2','1','3');
		$actual = SyndLib::array_collect($node->getOptions(), 'INFO_OPTION');
		$this->assertEquals($expected, $actual);

		$clone = $node->copy();
		$this->assertFalse($clone->nodeId == $node->nodeId);
		
		$actual = SyndLib::array_collect($clone->getOptions(), 'INFO_OPTION');
		$this->assertEquals($expected, $actual);
		
		$nodeOptions = array_values(SyndLib::array_collect($node->getOptions(), 'OPTION_NODE_ID'));
		$cloneOptions = array_values(SyndLib::array_collect($clone->getOptions(), 'OPTION_NODE_ID'));
		sort($nodeOptions);
		sort($cloneOptions);
		$this->assertFalse(reset($nodeOptions) == reset($cloneOptions));
		
		$clone->delete();
		$node->delete();
	}
}
