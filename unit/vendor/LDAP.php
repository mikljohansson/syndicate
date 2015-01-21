<?php
require_once 'PHPUnit2/Framework/TestCase.php';
require_once 'core/db/Database.class.inc';

class _vendor_LDAP extends PHPUnit2_Framework_TestCase {
	var $_db = null;
	
	function setUp() {
		global $synd_config;
		$this->_db = DatabaseManager::getConnection($synd_config['db']['ldap']);
	}
	
	function testGetAll() {
		$root = $this->_db->getAll('(objectClass=*)', 0, 1);
		$this->assertType('array', $root);
		$this->assertFalse(empty($root[0]['dn']));
	}

	function testGetCol() {
		$root = $this->_db->getAll('(objectClass=*)', 0, 1);

		$expected = array(reset(reset($root)));
		$actual = $this->_db->getCol('(objectClass=*)', 0, 0, 1);
		$this->assertEquals($expected, $actual);
		
		$column = reset(array_keys(reset($root)));
		$actual = $this->_db->getCol('(objectClass=*)', $column, 0, 1);
		$this->assertEquals($expected, $actual);
		
		$actual = $this->_db->getCol('(objectClass=*)', 'dn', 0, 1);
		$this->assertEquals($root[0]['dn'], $actual[0]);
	}

	function testGetAssoc() {
		$root = $this->_db->getAll('(objectClass=*)', 0, 1);
		$row = reset($root);
		
		$key = array_shift($row);
		if (is_array($key))
			$key = reset($key);
		
		$expected = array($key => reset($row));
		$actual = $this->_db->getAssoc('(objectClass=*)', 0, 1);
		$this->assertEquals($expected, $actual);
	}

	function testGetRow() {
		$root = $this->_db->getAll('(objectClass=*)', 0, 1);
		$expected = reset($root);
		$actual = $this->_db->getRow('(objectClass=*)');
		$this->assertEquals($expected, $actual);
		$this->assertFalse(empty($actual['dn']));
	}

	function testGetOne() {
		$root = $this->_db->getAll('(objectClass=*)', 0, 1);
		$expected = reset(reset($root));
		$actual = $this->_db->getOne('(objectClass=*)');
		$this->assertEquals($expected, $actual);
	}
	
	function testQuery() {
		$root = $this->_db->getAll('(objectClass=*)', 0, 1);
		$expected = reset($root);

		$query = $this->_db->query('(objectClass=*)', 0, 1);

		$actual = $query->fetchRow();
		$this->assertEquals($expected, $actual);
		$this->assertFalse(empty($actual['dn']));
		$this->assertFalse($query->fetchRow());
		
		$query = $this->_db->query('(objectClass=*)', 0, 1);
		$query->fetchInto($actual);
		$this->assertEquals($expected, $actual);
		$this->assertFalse(empty($actual['dn']));
		$query->fetchInto($actual);
		$this->assertFalse($actual);
	}
}
