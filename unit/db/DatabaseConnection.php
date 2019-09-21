<?php
require_once 'PHPUnit2/Framework/TestCase.php';

class _db_DatabaseConnection extends PHPUnit2_Framework_TestCase {
	function _exec($test) {
		global $synd_config, $synd_maindb;
		if (!isset($synd_config['db']['unit'])) {
			$synd_maindb->query('DELETE FROM synd_unit_test');
			call_user_func_array(array($this, "_$test"), array($synd_maindb));
		}
		else {
			foreach ($synd_config['db']['unit'] as $dsn) {
				$database = DatabaseManager::getConnection($dsn);
				$database->query('DELETE FROM synd_unit_test');
				call_user_func_array(array($this, "_$test"), array($database));
			}
		}
	}
	
	function testQuery()				{$this->_exec(__FUNCTION__);}
	function testRollback()				{$this->_exec(__FUNCTION__);}
	function testSavepoint()			{$this->_exec(__FUNCTION__);}
	function testSequence()				{$this->_exec(__FUNCTION__);}
	function testReplace()				{$this->_exec(__FUNCTION__);}
	function testInsert()				{$this->_exec(__FUNCTION__);}
	function testInsertAutoincrement()	{$this->_exec(__FUNCTION__);}
	function testLoad()					{$this->_exec(__FUNCTION__);}

	function testGetAll()				{$this->_exec(__FUNCTION__);}
	function testGetCol()				{$this->_exec(__FUNCTION__);}
	function testGetAssoc()				{$this->_exec(__FUNCTION__);}
	function testGetRow()				{$this->_exec(__FUNCTION__);}
	function testGetOne()				{$this->_exec(__FUNCTION__);}

	function testQuote()				{$this->_exec(__FUNCTION__);}
	function testTernary()				{$this->_exec(__FUNCTION__);}
	function testConcat()				{$this->_exec(__FUNCTION__);}

	function testTableStructure()		{$this->_exec(__FUNCTION__);}
	function testGetTableKeys()			{$this->_exec(__FUNCTION__);}
	function testDefaultValues()		{$this->_exec(__FUNCTION__);}

	function _testQuery($db) {
		$this->installTestData($db, 1);
		$this->installTestData($db, 2);
		$this->installTestData($db, 3);
		
		$sql = "
			SELECT t.NODE_ID FROM synd_unit_test t
			ORDER BY t.node_id";
		
		$expected = array(
			array('NODE_ID' => 'unit_test.1'),
			array('NODE_ID' => 'unit_test.2'),
			array('NODE_ID' => 'unit_test.3'));
		
		$result = $db->query($sql);
		
		$this->assertEquals($expected[0], $result->fetchRow());
		$this->assertEquals($expected[1], $result->fetchRow());
		$this->assertEquals($expected[2], $result->fetchRow());
		$this->assertEquals(false, $result->fetchRow());
		
		$result = $db->query($sql, 1, 10);
		$this->assertEquals($expected[1], $result->fetchRow());
		$this->assertEquals($expected[2], $result->fetchRow());
		$this->assertEquals(false, $result->fetchRow());

		$result = $db->query($sql, 1, 1);
		$this->assertEquals($expected[1], $result->fetchRow());
		$this->assertEquals(false, $result->fetchRow());
	}

	function _testRollback($db) {
		$sql = "SELECT t.node_id FROM synd_unit_test t";
		$this->assertNull($db->getOne($sql));

		$db->begin();
		$this->installTestData($db, 1);
		$this->assertEquals('unit_test.1', $db->getOne($sql));
		
		$db->rollback();
		$this->assertNull($db->getOne($sql));
	}
	
	function _testSavepoint($db) {
		$sql = "SELECT t.node_id FROM synd_unit_test t";
		$this->assertNull($db->getOne($sql));

		$db->begin();
		$this->installTestData($db, 1);
		$this->assertEquals(array('unit_test.1'), $db->getCol($sql));

		$db->begin();
		$this->installTestData($db, 2);
		$this->assertEquals(array('unit_test.1','unit_test.2'), $db->getCol($sql));

		$db->rollback();
		$this->assertEquals(array('unit_test.1'), $db->getCol($sql));
		
		$db->rollback();
		$this->assertNull($db->getOne($sql));
	}
	
	function _testSequence($db) {
		$id = $db->nextId('synd_unit_test.sequence_id');
		$this->assertEquals($id+1, $db->nextId('synd_unit_test.sequence_id'));
		$this->assertEquals($id+2, $db->nextId('synd_unit_test.sequence_id'));
		$this->assertEquals($id+3, $db->nextId('synd_unit_test.sequence_id'));
	}
	
	function _testReplace($db) {
		global $synd_maindb;

		$sql = "
			SELECT t.NODE_ID, t2.INFO_HEAD, t2.INFO_DESC
			FROM synd_unit_test t, synd_unit_test2 t2
			WHERE t.node_id = t2.node_id
			ORDER BY t.node_id";
		
		$this->installTestData($db, 1, 'Test', 'Clob');
		$expected = array(array('NODE_ID' => 'unit_test.1', 'INFO_HEAD' => 'Test', 'INFO_DESC' => 'Clob'));
		$this->assertEquals($expected, $db->getAll($sql));

		$expected = array(array('NODE_ID' => 'unit_test.1', 'INFO_HEAD' => null, 'INFO_DESC' => 'Clob2'));
		$db->replace('synd_unit_test', $expected[0]);
		$db->replace('synd_unit_test2', $expected[0]);
		$this->assertEquals($expected, $db->getAll($sql));
		
		$expected[] = array('NODE_ID' => 'unit_test.2', 'INFO_HEAD' => 'Test3', 'INFO_DESC' => 'Clob3');
		$db->replace('synd_unit_test', $expected[1]);
		$db->replace('synd_unit_test2', $expected[1]);
		$this->assertEquals($expected, $db->getAll($sql));
		
		$db->delete('synd_unit_test', $expected[1]);
		$db->delete('synd_unit_test2', $expected[1]);
		unset($expected[1]);
		$this->assertEquals($expected, $db->getAll($sql));
	}

	function _testInsert($db) {
		$sql = "
			SELECT t.NODE_ID
			FROM synd_unit_test t
			ORDER BY t.node_id";

		$expected = array(array('NODE_ID' => 'unit_test.1'));
		$db->insert('synd_unit_test', $expected[0]);
		$this->assertEquals($expected, $db->getAll($sql));
		
		$expected[] = array('NODE_ID' => 'unit_test.2');
		$db->insert('synd_unit_test', $expected[1]);
		$this->assertEquals($expected, $db->getAll($sql));
	}

	function _testInsertAutoincrement($db) {
		$data = $db->insert('synd_unit_test', array('NODE_ID' => 'unit_test.1'));
		$this->assertFalse(SyndLib::isError($data));
		$this->assertFalse(empty($data['SEQUENCE_ID']));
	}

	function _testLoad($db) {
		$sql = "
			SELECT t.NODE_ID
			FROM synd_unit_test t
			ORDER BY t.node_id";
		
		$expected = array(
			array('NODE_ID' => 'unit_test.1'),
			array('NODE_ID' => 'unit_test.2'),
			array('NODE_ID' => 'unit_test.3'));
		$db->load('synd_unit_test', $expected);
			
		$actual = $db->getAll($sql);
		$this->assertEquals($expected, $actual);
	}

	function _testGetAll($db) {
		$this->installTestData($db, 1);
		$this->installTestData($db, 2);
		$this->installTestData($db, 3);
		
		$sql = "
			SELECT t.NODE_ID
			FROM synd_unit_test t
			ORDER BY t.node_id";
		
		$expected = array(
			array('NODE_ID' => 'unit_test.1'),
			array('NODE_ID' => 'unit_test.2'),
			array('NODE_ID' => 'unit_test.3'));
		$actual = $db->getAll($sql);
		$this->assertEquals($expected, $actual);
		
		$expected = array(
			array('NODE_ID' => 'unit_test.2'),
			array('NODE_ID' => 'unit_test.3'));
		$actual = $db->getAll($sql, 1, 50);
		$this->assertEquals($expected, $actual);
		
		$expected = array(
			array('NODE_ID' => 'unit_test.2'));
		$actual = $db->getAll($sql, 1, 1);
		$this->assertEquals($expected, $actual);
		
		$expected = array(
			array('NODE_ID' => 'unit_test.3'));
		$actual = $db->getAll($sql, 2, 50);
		$this->assertEquals($expected, $actual);
	}

	function _testGetCol($db) {
		$this->installTestData($db, 1);
		$this->installTestData($db, 2);
		$this->installTestData($db, 3);
		
		$sql = "
			SELECT 'Test' AS COL1, t.NODE_ID
			FROM synd_unit_test t
			ORDER BY t.node_id";
		
		$expected = array('unit_test.1', 'unit_test.2', 'unit_test.3');
		$actual = $db->getCol($sql, 1);
		$this->assertEquals($expected, $actual);
		
		$expected = array('unit_test.2', 'unit_test.3');
		$actual = $db->getCol($sql, 'NODE_ID', 1, 50);
		$this->assertEquals($expected, $actual);
	}
	
	function _testGetAssoc($db) {
		$this->installTestData($db, 1, 'Test1');
		$this->installTestData($db, 2, 'Test2');
		$this->installTestData($db, 3, 'Test3');
		
		$sql = "
			SELECT t2.NODE_ID, t2.INFO_HEAD
			FROM synd_unit_test2 t2
			ORDER BY t2.node_id";
		
		$expected = array(
			'unit_test.1' => 'Test1', 
			'unit_test.2' => 'Test2', 
			'unit_test.3' => 'Test3');
		$actual = $db->getAssoc($sql);
		$this->assertEquals($expected, $actual);
	}
	
	function _testGetRow($db) {
		$this->installTestData($db, 1);
		$this->installTestData($db, 2);
		
		$sql = "
			SELECT t.NODE_ID
			FROM synd_unit_test t
			ORDER BY t.node_id";
		
		$expected = array('NODE_ID' => 'unit_test.1');
		$actual = $db->getRow($sql);
		$this->assertEquals($expected, $actual);
	}

	function _testGetOne($db) {
		$this->installTestData($db, 1);
		
		$sql = "
			SELECT t.NODE_ID
			FROM synd_unit_test t
			ORDER BY t.node_id";
		
		$expected = 'unit_test.1';
		$actual = $db->getOne($sql);
		$this->assertEquals($expected, $actual);
	}

	function _testQuote($db) {
		$expected = "Foo'Bar\"";
		
		$data = array('NODE_ID' => 'unit_test.1');
		$data2 = array('NODE_ID' => 'unit_test.1', 'INFO_HEAD' => $expected);
		$db->insert('synd_unit_test', $data);
		$db->insert('synd_unit_test2', $data2);
		
		$sql = "SELECT t2.info_head FROM synd_unit_test2 t2";
		$actual = $db->getOne($sql);
		$this->assertEquals($expected, $actual);
		
		$expected = 1.2;
		$actual = $db->quote(1.2);
		$this->assertType('float', $actual);
		$this->assertEquals($expected, $actual);

		$expected = array("'Foo'");
		$actual = $db->quote(array('Foo'));
		$this->assertEquals($expected, $actual);
		
		$expected = 'NULL';
		$actual = $db->quote(null);
		$this->assertEquals($expected, $actual);
		$actual = $db->quote(null);
		$this->assertEquals($expected, $actual);
		
		$expected = 123;
		$actual = $db->quote(123);
		$this->assertType('integer', $actual);
		$this->assertEquals($expected, $actual);

		$expected = 123;
		$actual = $db->quote(123);
		$this->assertType('integer', $actual);
		$this->assertEquals($expected, $actual);
	}

	function _testTernary($db) {
		$this->installTestData($db, 1);
		$this->installTestData($db, 2);

		$sql = "
			SELECT ".$db->ternary("t.node_id = 'unit_test.1'","'Test1'","'Test2'")."
			FROM synd_unit_test t
			ORDER BY t.node_id";
		$actual = $db->getCol($sql);
		$this->assertEquals(array('Test1','Test2'), $actual);
	}

	function _testConcat($db) {
		$sql = "
			SELECT ".$db->concat(array("'Foo'","'-'","'Bar'"))."
			FROM synd_unit_test t";
		$this->installTestData($db, 1);
		$this->assertEquals('Foo-Bar', $db->getOne($sql));
	}

	function _testTableStructure($db) {
		$structure = $db->getTableStructure('synd_unit_test');

		$data = array(
			'SEQUENCE_ID' => 'Test');
			
		$expected = array(
			'NODE_ID',		// Value is missing
			'SEQUENCE_ID',	// Value is non-numeric
			);
		
		$actual = $structure->validate($data);
		$this->assertEquals($expected, array_keys($actual));
	}
	
	function testStructureValidation() {
		// Test string types
		require_once 'core/db/TableMetadata.class.inc';
		$structure = new TableMetadata('synd_unit_test');
		$structure->addColumn(new DatabaseStringColumn('COL1', true, null, 255, true));	// Test width
		$structure->addColumn(new DatabaseStringColumn('COL2', true, null, 255, false));	// Test binary data
		$structure->addColumn(new DatabaseStringColumn('COL3', false, null));				// Test required field
		$structure->addColumn(new DatabaseStringColumn('COL4', false, 'Test'));			// Test required field
		
		$data = array(
			'COL1' => str_repeat('a', 256),
			'COL2' => "abc \0 def",
			);
		$expected = array(
			'COL1', 
			'COL2', 
			'COL3',
			);
		
		$actual = $structure->validate($data);
		$this->assertEquals($expected, array_keys($actual));

		// Test numeric types
		$structure = new TableMetadata('synd_unit_test');
		$structure->addColumn(new DatabaseIntegerColumn('COL1', true, null, 8, true));	// Test bitwidth
		$structure->addColumn(new DatabaseIntegerColumn('COL2', true, null, 8, true));	// Test bitwidth
		$structure->addColumn(new DatabaseIntegerColumn('COL3', true, null, 8));			// Test bitwidth
		$structure->addColumn(new DatabaseIntegerColumn('COL4', true, null, 8));			// Test bitwidth
		$structure->addColumn(new DatabaseIntegerColumn('COL5', true, null, 32, true));	// Test unsigned
		$structure->addColumn(new DatabaseIntegerColumn('COL6', true, null));				// Test non numeric
		$structure->addColumn(new DatabaseFloatColumn('COL7', true, null, true));			// Test unsigned
		$structure->addColumn(new DatabaseFloatColumn('COL8', true, null));				// Test non numeric
		
		$data = array(
			'COL1' => 256,
			'COL2' => 255,
			'COL3' => -129,
			'COL4' => -128,
			'COL5' => -1,
			'COL6' => 'Test',
			'COL7' => -1,
			'COL8' => 'Test',
			);
		$expected = array(
			'COL1', 
			'COL3',
			'COL5',
			'COL6',
			'COL7',
			'COL8',
			);
		
		$actual = $structure->validate($data);
		$this->assertEquals($expected, array_keys($actual));

		// Test date types
		$structure = new TableMetadata('synd_unit_test');
		$structure->addColumn(new DatabaseDatetimeColumn('COL1', true, null));	// Test correct
		$structure->addColumn(new DatabaseDatetimeColumn('COL2', true, null));	// Test invalid format

		$data = array(
			'COL1' => '2005-01-01 00:00:00',
			'COL2' => '2005-01 00:00:00',
			);
		$expected = array(
			'COL2',
			);
		
		$actual = $structure->validate($data);
		$this->assertEquals($expected, array_keys($actual));
	}
	
	function _testGetTableKeys($db) {
		$actual = $db->getTableKeys('synd_unit_test2');
		$this->assertEquals(1, count($actual));
	}
	
	function _testDefaultValues($db) {
		$expected = array(
			'NODE_ID'	=> null,
			'INFO_HEAD' => 'Test',
			'INFO_DESC' => null,
			);
		
		$structure = $db->getTableStructure('synd_unit_test2');
		$actual = $structure->getDefaultValues();
		
		$this->assertEquals($expected, $actual);
	}
	
	function installTestData($db, $id = 1, $head = null, $desc = null) {
		$sql = "
			INSERT INTO synd_unit_test
			(node_id)
			VALUES
			('unit_test.$id')";
		$this->assertFalse(SyndLib::isError($db->query($sql)));

		$sql = "
			INSERT INTO synd_unit_test2
			(node_id, info_head, info_desc)
			VALUES
			('unit_test.$id', ".$db->quote($head).", ".$db->quote($desc).")";
		$this->assertFalse(SyndLib::isError($db->query($sql)));
	}
}
