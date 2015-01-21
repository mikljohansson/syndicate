<?php
require_once 'unit/SyndNodeTestCase.class.inc';

class _model_CachedStorage extends SyndNodeTestCase {
	function setUp() {
		require_once 'core/lib/CacheStrategy.class.inc';
		require_once 'core/model/storage/CachedStorage.class.inc';
	}

	function testConcurrencyCacheMiss() {
		$storage = SyndNodeLib::getDefaultStorage('unit_test');
		$persistent = $storage->getPersistentStorage();
		$database = $persistent->getDatabase();
		
		$cached1 = new CachedStorage($persistent, CacheStrategyManager::factory(), 5);
		$cached2 = new CachedStorage($persistent, CacheStrategyManager::factory(), 5);
		
		// Place node to database only
		$node = $persistent->factory('unit_test');
		$node->save();
		$node->flush();
		
		// Edit node1 but do not flush
		$node1 = $cached1->getInstance($node->nodeId);
		$node1->_children = 'Test1';
		$node1->save();

		// Save and flush node2 to cache
		$node2 = $cached2->getInstance($node->nodeId);
		$node2->_children = 'Test2';
		$node2->save();
		$node2->flush();
		
		// Check cache
		$node3 = $cached2->getInstance($node->nodeId);
		$this->assertTrue(isset($node3->_children) && 'Test2' == $node3->_children);
		
		// Flush node1 should result in conflict detection and cache cleared for node->nodeId
		$node1->flush();
		
		$sql = "
			DELETE FROM synd_unit_test
			WHERE node_id = ".$database->quote($node->nodeId);
		$this->assertFalse(SyndLib::isError($database->query($sql)));
		
		// Reload node3 and check _children
		$node3 = $cached2->getInstance($node->nodeId);
		$this->assertNull($node3);
	}
	
//	function testConcurrencyCacheHit() {
//		$storage = SyndNodeLib::getDefaultStorage('unit_test');
//		$persistent = $storage->getPersistentStorage();
//		$database = $persistent->getDatabase();
//
//		$cached1 = new CachedStorage($persistent, CacheStrategyManager::factory(), 5);
//		$cached2 = new CachedStorage($persistent, CacheStrategyManager::factory(), 5);
//		
//		// Place node to cache
//		$node = $cached1->factory('unit_test');
//		$node->save();
//		$node->flush();
//		
//		// Signal node1 as edited but not flushed
//		$node1 = $cached1->getInstance($node->nodeId);
//		$node1->_children = 'Test1';
//		$node1->save();
//		
//		// Save and flush node2 to cache
//		$node2 = $cached2->getInstance($node->nodeId);
//		$node2->_children = 'Test2';
//		$node2->save();
//		$node2->flush();
//		
//		// Check cache
//		$node3 = $cached2->getInstance($node->nodeId);
//		$this->assertTrue(isset($node3->_children) && 'Test2' == $node3->_children);
//		
//		// Flush node1 should result in conflict detection and cache cleared for node->nodeId
//		$node1->flush();
//		
//		$sql = "
//			DELETE FROM synd_unit_test
//			WHERE node_id = ".$database->quote($node->nodeId);
//		$database->query($sql);
//		
//		// Reload node3 and check _children
//		$node3 = $cached2->getInstance($node->nodeId);
//		$this->assertNull($node3);
//	}
//
//	function testConcurrencyCacheHit2() {
//		$storage = SyndNodeLib::getDefaultStorage('unit_test');
//		$persistent = $storage->getPersistentStorage();
//		$database = $persistent->getDatabase();
//
//		$cached1 = new CachedStorage($persistent, CacheStrategyManager::factory(), 5);
//		$cached2 = new CachedStorage($persistent, CacheStrategyManager::factory(), 5);
//		
//		// Place node to cache
//		$node = $cached1->factory('unit_test');
//		$node->save();
//		$node->flush();
//		
//		// Edited node1
//		$node1 = $cached1->getInstance($node->nodeId);
//		$node1->_children = 'Test1';
//		
//		// Signal node2 as edited
//		$node2 = $cached2->getInstance($node->nodeId);
//		$node2->_children = 'Test2';
//		$node2->save();
//		
//		// Flush nodes should result in conflict detection and cache cleared for node->nodeId
//		$node1->save();
//		@$node1->flush();
//		$node2->flush();
//
//		$sql = "
//			DELETE FROM synd_unit_test
//			WHERE node_id = ".$database->quote($node->nodeId);
//		$database->query($sql);
//		
//		// Reload node3 and check _children
//		$node3 = $cached2->getInstance($node->nodeId);
//		$this->assertNull($node3);
//	}
}
