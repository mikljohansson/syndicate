<?php
require_once 'PHPUnit2/Framework/TestCase.php';

class _vendor_TimeEdit extends PHPUnit2_Framework_TestCase {
	function setUp() {
		require_once 'core/lib/vendor/TimeEdit.class.inc';
	}
	
	function testConnection() {
		$te = connect_timeedit();
		$this->assertNotNull($te->getSessionId());
	}
	
	function testDomains() {
		$te = connect_timeedit();
		$domains = $te->getDomains();
		$this->assertFalse(empty($domains));
		
		$domain = $domains[key($domains)];
		$this->assertNotNull($domain->getName());
		$this->assertEquals(key($domains), $domain->getID());
	}
	
	function testTypes() {
		$te = connect_timeedit();
		$domains = $te->getDomains();
		$domain = $domains[key($domains)];
		
		$types = $domain->getTypes();
		$this->assertFalse(empty($types));

		$type = $types[key($types)];
		$this->assertNotNull($type->getSignature());
		$this->assertEquals(key($types), $type->getID());
	}
	
	function testObjectSearch() {
		$te = connect_timeedit();
		$domains = $te->getDomains();
		$domain = $domains[key($domains)];

		$types = $domain->getTypes();
		$type = $types[key($types)];

		$objects = $type->searchBySignature('*');
		$this->assertFalse(empty($objects));
		if (empty($objects)) return;

		$object = $objects[key($objects)];
		$this->assertNotNull($object->getSignature());
		$this->assertNotNull($object->getName());
		$this->assertEquals(key($objects), $object->getID());
	}
}