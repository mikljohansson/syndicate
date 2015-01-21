<?php
require_once 'PHPUnit2/Framework/TestCase.php';

class _design_TableWidget extends PHPUnit2_Framework_TestCase {
	function setUp() {
		require_once 'design/gui/TableWidget.class.inc';
	}
	
	function testRowspan() {
		$widget = new TableWidget(array('class'=>'Example'));
		$widget->add(new TableWidgetCell(0,0,'Test',array('rowspan' => 2)));
		$widget->add(new TableWidgetCell(0,1,'Test2',null,'th'));
		
		$expected = file_get_contents(dirname(__FILE__).'/_table/table-001-rowspan.html');
		$actual = $widget->toString();
		
		$this->assertEquals(preg_replace('/\s*\n\s*/', '', $expected), preg_replace('/\s*\n\s*/', '', $actual));
	}

	function testColspan() {
		$widget = new TableWidget();
		$widget->add(new TableWidgetCell(0,0,'Test',array('colspan' => 2)));
		$widget->add(new TableWidgetCell(1,0,'Test2'));
		
		$expected = file_get_contents(dirname(__FILE__).'/_table/table-002-colspan.html');
		$actual = $widget->toString();
		
		$this->assertEquals(preg_replace('/\s*\n\s*/', '', $expected), preg_replace('/\s*\n\s*/', '', $actual));
	}

	function testRowspanConflict() {
		$widget = new TableWidget();
		$widget->add(new TableWidgetCell(0,0,'Test'));
		$widget->add(new TableWidgetCell(1,0,'Test2',array('rowspan'=>2)));
		$widget->add(new TableWidgetCell(1,0,'Test3'));
		$widget->add(new TableWidgetCell(1,0,'Test4'));

		$expected = file_get_contents(dirname(__FILE__).'/_table/table-003-rowspan-conflict.html');
		$actual = $widget->toString();
		$this->assertEquals(preg_replace('/\s*\n\s*/', '', $expected), preg_replace('/\s*\n\s*/', '', $actual));


		$widget = new TableWidget();
		$widget->add(new TableWidgetCell(0,1,'Test'));
		$widget->add(new TableWidgetCell(0,1,'Test2'));
		$widget->add(new TableWidgetCell(0,2,'Test3'));
		$widget->add(new TableWidgetCell(1,0,'Test4',array('rowspan'=>3)));
		$widget->add(new TableWidgetCell(1,1,'Test5',array('rowspan'=>2)));

		$expected = file_get_contents(dirname(__FILE__).'/_table/table-004-rowspan-conflict.html');
		$actual = $widget->toString();
		$this->assertEquals(preg_replace('/\s*\n\s*/', '', $expected), preg_replace('/\s*\n\s*/', '', $actual));


		$widget = new TableWidget();
		$widget->add(new TableWidgetCell(0,0,'Test',array('rowspan'=>2)));
		$widget->add(new TableWidgetCell(1,0,'Test2',array('rowspan'=>2)));

		$expected = file_get_contents(dirname(__FILE__).'/_table/table-005-rowspan-conflict.html');
		$actual = $widget->toString();
		$this->assertEquals(preg_replace('/\s*\n\s*/', '', $expected), preg_replace('/\s*\n\s*/', '', $actual));
	}
	
	function testDefaultAttributes() {
		$widget = new TableWidget();
		$widget->setDefaultAttribute('class', 'EmptyCell');
		$widget->setDefaultAttributesCallback(array($this, '_callback_default_attributes'));
		$widget->add(new TableWidgetCell(0,0,'Test',array('colspan' => 2)));
		$widget->add(new TableWidgetCell(1,0,'Test2'));
		
		$expected = file_get_contents(dirname(__FILE__).'/_table/table-006-default-attributes.html');
		$actual = $widget->toString();
		
		$this->assertEquals(preg_replace('/\s*\n\s*/', '', $expected), preg_replace('/\s*\n\s*/', '', $actual));
	}
	
	function _callback_default_attributes($widget, $row, $col) {
		return array_merge(array('row'=>$row,'col'=>$col), $widget->getDefaultAttributes());
	}
}
