<?php
require_once 'PHPUnit2/Framework/TestCase.php';

class _vendor_Raindance extends PHPUnit2_Framework_TestCase {
	function testSearchCostcenter() {
		$inventory = Module::getInstance('inventory');
		$plugin = $inventory->loadPlugin('raindance');
		
		$database = $plugin->getDatabase();
		$sql = "
			SELECT k.id FROM xobjkst k
			WHERE 
				(k.fomdatum IS NULL OR k.fomdatum <= SYSDATE) AND 
				(k.tomdatum IS NULL OR k.tomdatum >= SYSDATE)";
		$id = $database->getOne($sql);
		$this->assertNotNull($id);
		
		$collections = SyndLib::runHook('search_costcenter', $id);
		$this->assertFalse(empty($collections));
		
		if (count($collections)) {
			$collection = SyndType::factory('composite_collection', $collections);
			$contents = $collection->getContents(0, 10);
			$this->assertTrue(isset($contents["raindance.costcenter.$id"]));

			$found = SyndLib::runHook('find_costcenter', $id);
			$this->assertNotNull($found);
			
			$costcenter = SyndLib::getInstance("raindance.costcenter.$id");
			$this->assertNotNull($costcenter);
			
			$this->assertEquals($found, $costcenter);
		}
	}

	function testSearchProject() {
		$inventory = Module::getInstance('inventory');
		$plugin = $inventory->loadPlugin('raindance');
		
		$database = $plugin->getDatabase();
		$sql = "
			SELECT p.id FROM xobjproj p
			WHERE 
				(p.fomdatum IS NULL OR p.fomdatum <= SYSDATE) AND 
				(p.tomdatum IS NULL OR p.tomdatum >= SYSDATE)";
		$id = $database->getOne($sql);
		$this->assertNotNull($id);
		
		$collections = SyndLib::runHook('search_project', $id);
		$this->assertFalse(empty($collections));
		
		if (count($collections)) {
			$collection = SyndType::factory('composite_collection', $collections);
			$contents = $collection->getContents(0, 10);
			$this->assertTrue(isset($contents["raindance.project.$id"]));

			$found = SyndLib::runHook('find_project', $id);
			$this->assertNotNull($found);

			$project = SyndLib::getInstance("raindance.project.$id");
			$this->assertNotNull($project);

			$this->assertEquals($found, $project);
		}
	}
}
