<?php
require_once 'unit/module/SyncMLTestcase.class.inc';
require_once 'core/model/ISyncable.class.inc';
require_once 'core/model/syncml/message.class.inc';

class _modules_SyncMLSyncronize extends SyncMLTestcase {
	function testEnabled() {
		$syncml = Module::getInstance('syncml');
		$this->assertNotNull($syncml);
	}
	
	/**
	 * Client alerts it wants a normal sync and sends its Last and 
	 * Next anchors. Servers stored Last anchor matches the clients
	 * Last anchor so we respond normally by sending "Alert 200"
	 */
	function testAlertNormalSync() {
		$message = new synd_syncml_message(file_get_contents(dirname(__FILE__).'/_syncml/syncml-004-alert-normal.xml'));
		$document = $message->getDocument();
		$last = $document->selectSingleNode('SyncBody', 'Alert', 'Item', 'Meta', 'Anchor', 'Last');
		
		$this->assertFalse($last->isNull());
		$persistent = $message->getPersistentSession();
		$persistent->setLastAnchor(_SyncableUnittestCollection::instance(), $last->getContent());
		
		$actual = $message->getResponse();
		$expected = new synd_syncml_message(file_get_contents(dirname(__FILE__).'/_syncml/syncml-004-response-normal.xml'));
		
		$this->assertEquals($expected, $actual);
		//$this->_diff($expected->toString(), $actual->toString());
		
		$message->delete();
	}

	/**
	 * Client does not provide necessary credentials for collection
	 */
	function testAlertNormalUnauthorized() {
		$message = new synd_syncml_message(file_get_contents(dirname(__FILE__).'/_syncml/syncml-018-alert-unauthorized.xml'));
		$actual = $message->getResponse();
		$expected = new synd_syncml_message(file_get_contents(dirname(__FILE__).'/_syncml/syncml-018-response-credentials-missing.xml'));
		
		$this->assertEquals($expected, $actual);
		//$this->_diff($expected->toString(), $actual->toString());

		$message->delete();
	}
	
	/**
	 * 404 collection not found
	 */
	function testAlertNormalNotFound() {
		$message = new synd_syncml_message(file_get_contents(dirname(__FILE__).'/_syncml/syncml-019-alert.xml'));
		$actual = $message->getResponse();
		$expected = new synd_syncml_message(file_get_contents(dirname(__FILE__).'/_syncml/syncml-019-response-notfound.xml'));
		
		$this->assertEquals($expected, $actual);
		//$this->_diff($expected->toString(), $actual->toString());

		$message->delete();
	}

	/**
	 * Client alerts it wants a normal sync and sends its Last and 
	 * Next anchors. Servers stored Last anchor does NOT match clients
	 * Last anchor so we force a slow sync by sending "Alert 201"
	 */
	function testAlertNormalSyncSlow() {
		$message = new synd_syncml_message(file_get_contents(dirname(__FILE__).'/_syncml/syncml-004-alert-normal.xml'));
		$actual = $message->getResponse();
		$expected = new synd_syncml_message(file_get_contents(dirname(__FILE__).'/_syncml/syncml-004-response-slow.xml'));
		
		$this->assertEquals($expected, $actual);
		//$this->_diff($expected->toString(), $actual->toString());

		$message->delete();
	}
	
	/**
	 * Client alerts it wants a slow sync and sends its Next anchor. 
	 * Server responds with a "Alert 201"
	 */
	function testAlertSlowSync() {
		$message = new synd_syncml_message(file_get_contents(dirname(__FILE__).'/_syncml/syncml-005-alert-slow.xml'));
		$actual = $message->getResponse();
		
		$expected = new synd_syncml_message(file_get_contents(dirname(__FILE__).'/_syncml/syncml-005-response.xml'));
		$this->assertEquals($expected, $actual);
		$this->_diff($expected->toString(), $actual->toString());

		$message->delete();
	}

	function testAdd() {
		$message = new synd_syncml_message(file_get_contents(dirname(__FILE__).'/_syncml/syncml-009-add.xml'));
		$actual = $message->getResponse();
		$expected = new synd_syncml_message(file_get_contents(dirname(__FILE__).'/_syncml/syncml-009-response.xml'));
		$this->assertEquals($expected, $actual);
		
		$document = $message->getDocument();
		$data = $document->selectSingleNode('SyncBody', 'Sync', 'Add', 'Item', 'Data');
		$ical = $data->getMemento('text/x-vcalendar');
		$this->assertEquals('syndicalendar', strtolower(get_class($ical)));
		
		$collection = _SyncableUnittestCollection::instance();
		$this->assertEquals($ical, $collection->_appended[0]);

		$persistent = $message->getPersistentSession();
		$this->assertEquals('_syncml._unit_test1', $persistent->getGlobalId($collection, '1'));

		$message->delete();
	}

	function testReplace() {
		$collection = _SyncableUnittestCollection::instance();
		$instance = new _SyncableUnittestInstance('_syncml._unit_test');

		$message = new synd_syncml_message(file_get_contents(dirname(__FILE__).'/_syncml/syncml-007-replace.xml'));
		$persistent = $message->getPersistentSession();
		$persistent->setGlobalId($collection, '1', '_syncml._unit_test1');
		$actual = $message->getResponse();
		
		$expected = new synd_syncml_message(file_get_contents(dirname(__FILE__).'/_syncml/syncml-007-response.xml'));
		$this->assertEquals($expected, $actual);

		$document = $message->getDocument();
		$data = $document->selectSingleNode('SyncBody', 'Sync', 'Replace', 'Item', 'Data');
		$ical = $data->getMemento('text/x-vcalendar');
		$this->assertEquals('syndicalendar', strtolower(get_class($ical)));
		
		$this->assertEquals($ical, $collection->_appended[0]);
		
		$persistent = $message->getPersistentSession();
		$this->assertEquals('_syncml._unit_test1', $persistent->getGlobalId($collection, '1'));
		$this->assertEquals('_syncml._unit_test2', $persistent->getGlobalId($collection, '2'));

		$message->delete();
	}
		
	function testDelete() {
		$collection = _SyncableUnittestCollection::instance();

		$message = new synd_syncml_message(file_get_contents(dirname(__FILE__).'/_syncml/syncml-010-delete.xml'));
		$persistent = $message->getPersistentSession();
		$persistent->setGlobalId($collection, '1', '_syncml._unit_test');
		$this->assertEquals('_syncml._unit_test', $persistent->getGlobalId($collection, '1'));
		$actual = $message->getResponse();
		
		$expected = new synd_syncml_message(file_get_contents(dirname(__FILE__).'/_syncml/syncml-010-response.xml'));
		$this->assertEquals($expected, $actual);
		$this->assertEquals($collection->getInstance('_syncml._unit_test'), $collection->_removed[0]);
		$this->assertNull($persistent->getGlobalId($collection, '1'));
		
		$message->delete();
	}
	
	function testDeleteOutbound() {
		$collection = _SyncableUnittestCollection::instance();

		$message = new synd_syncml_message(file_get_contents(dirname(__FILE__).'/_syncml/syncml-010-response.xml'));
		$persistent = $message->getPersistentSession();
		$persistent->setGlobalId($collection, '1', '_syncml._unit_test');
		
		// Create and store the outgoing delete in the session
		$response = new synd_syncml_message();
		$response->nextCommandId(); // Simulate the Sync element
		
		$delete = synd_syncml_delete::factory($response, $collection, '1');
		$session = $message->getSession();
		$session->setCommand($message->getMessageId(), $delete->getCommandId(), $delete);
		
		$this->assertEquals('_syncml._unit_test', $persistent->getGlobalId($collection, '1'));
		$actual = $message->getResponse();
		
		$this->assertNull($persistent->getGlobalId($collection, '1'));
		
		$message->delete();
	}
	
	function testMap() {
		$collection = _SyncableUnittestCollection::instance();
		$instance = new _SyncableUnittestInstance('case._unit_test', null, true);

		// Test unauthorized 
		$buffer = file_get_contents(dirname(__FILE__).'/_syncml/syncml-011-map.xml');
		$message = new synd_syncml_message(str_replace('__TARGET__', 'case._unit_test', $buffer));

		$actual = $message->getResponse();
		$expected = new synd_syncml_message(file_get_contents(dirname(__FILE__).'/_syncml/syncml-011-failed.xml'));
		$this->assertEquals($expected, $actual);
		//$this->_diff($expected, $actual);
		
		$message->delete();

		// Test authorized
		$buffer = file_get_contents(dirname(__FILE__).'/_syncml/syncml-011-map.xml');
		$message = new synd_syncml_message(str_replace('__TARGET__', '_syncml._unit_test', $buffer));
		$actual = $message->getResponse();
		
		$expected = new synd_syncml_message(file_get_contents(dirname(__FILE__).'/_syncml/syncml-011-response.xml'));
		$this->assertEquals($expected, $actual);
		//$this->_diff($expected, $actual);

		$persistent = $message->getPersistentSession();
		$this->assertEquals('_syncml._unit_test', $persistent->getGlobalId($collection, '1'));

		$message->delete();
	}

	function testSlowSync() {
		require_once 'core/lib/calendar/VCalendar.class.inc';
		
		// Session setup (Initialization message)
		$message = new synd_syncml_message(file_get_contents(dirname(__FILE__).'/_syncml/syncml-012-slow1-init.xml'));

		$persistent = $message->getPersistentSession();
		$collection = _SyncableUnittestCollection::instance();

		$actual = $message->getResponse();
		$expected = new synd_syncml_message(file_get_contents(dirname(__FILE__).'/_syncml/syncml-012-slow1-response.xml'));
		$this->assertEquals($expected, $actual);
		//$this->_diff($expected, $actual);
		
		// New item to Add to client
		$ical = VCalendar::parse(file_get_contents(dirname(__FILE__).'/_syncml/syncml-012-slow2-memento.ics'));
		$collection->_contents[] = new _SyncableUnittestInstance('_syncml._unit_test2', $ical);

		// Item to be Replaced on server and back to client
		$collection->_contents[] = new _SyncableUnittestInstance('_syncml._unit_test1', $ical);
		$persistent->setGlobalId($collection, 1, '_syncml._unit_test1');
		
		// Modifications from client
		$message = new synd_syncml_message(file_get_contents(dirname(__FILE__).'/_syncml/syncml-012-slow2-modifications.xml'));
		$actual = $message->getResponse();
		
		$expected = new synd_syncml_message(file_get_contents(dirname(__FILE__).'/_syncml/syncml-012-slow2-response.xml'));
		$this->assertEquals($expected, $actual);
		
		//debug(str_replace("\r\n", "\\r\\n\r\n", $expected->toString()));
		//debug(str_replace("\r\n", "\\r\\n\r\n", $actual->toString()));
		//$this->_diff($expected->toString(), $actual->toString());

		// Map message
		$message = new synd_syncml_message(file_get_contents(dirname(__FILE__).'/_syncml/syncml-012-slow3-map.xml'));
		$actual = $message->getResponse();
		
		$expected = new synd_syncml_message(file_get_contents(dirname(__FILE__).'/_syncml/syncml-012-slow3-response.xml'));
		$this->assertEquals($expected, $actual);
		//$this->_diff($expected, $actual);

		$message->delete();
	}

	function testNormalSync() {
		require_once 'core/lib/calendar/VCalendar.class.inc';
		
		// Session setup (Initialization message)
		$message = new synd_syncml_message(file_get_contents(dirname(__FILE__).'/_syncml/syncml-013-normal1-init.xml'));

		$collection = _SyncableUnittestCollection::instance();
		$persistent = $message->getPersistentSession();
		$persistent->setLastAnchor($collection, '20050101T142005Z');
		$persistent->setGlobalId($collection, 3, 'case._unit_test_deleted');

		$actual = $message->getResponse();
		$expected = new synd_syncml_message(file_get_contents(dirname(__FILE__).'/_syncml/syncml-013-normal1-response.xml'));
		$this->assertEquals($expected, $actual);
		//$this->_diff($expected, $actual);
		
		// New item to Add to client
		$ical = VCalendar::parse(file_get_contents(dirname(__FILE__).'/_syncml/syncml-013-normal2-memento.ics'));
		$collection->_contents[] = new _SyncableUnittestInstance('_syncml._unit_test2', $ical);

		// Item to be Replaced on server and back to client
		$collection->_contents[] = new _SyncableUnittestInstance('_syncml._unit_test1', $ical);
		$persistent->setGlobalId($collection, 1, '_syncml._unit_test1');
		
		// Modifications from client
		$message = new synd_syncml_message(file_get_contents(dirname(__FILE__).'/_syncml/syncml-013-normal2-modifications.xml'));
		$actual = $message->getResponse();

		//Modification from server
		$expected = new synd_syncml_message(file_get_contents(dirname(__FILE__).'/_syncml/syncml-013-normal2-response.xml'));
		$document = $expected->getDocument();
		$delete = $document->selectSingleNode('SyncBody', 'Sync', 'Delete');
		$delete->_collection = $collection->getName();
		$this->assertEquals($expected, $actual);

		//debug($expected->toString());
		//debug($actual->toString());
		//$this->_diff($expected->toString(), $actual->toString());

		// Map message
		$message = new synd_syncml_message(file_get_contents(dirname(__FILE__).'/_syncml/syncml-013-normal3-map.xml'));
		$actual = $message->getResponse();
		
		$expected = new synd_syncml_message(file_get_contents(dirname(__FILE__).'/_syncml/syncml-013-normal3-response.xml'));
		$this->assertEquals($expected, $actual);
		//$this->_diff($expected, $actual);

		$message->delete();
	}
	
	function testOneWayClient() {
		require_once 'core/lib/calendar/VCalendar.class.inc';
		$collection = _SyncableUnittestCollection::instance();
		
		// Session setup (Initialization message)
		$message = new synd_syncml_message(file_get_contents(dirname(__FILE__).'/_syncml/syncml-014-oneway-client1-init.xml'));
		$actual = $message->getResponse();

		$expected = new synd_syncml_message(file_get_contents(dirname(__FILE__).'/_syncml/syncml-014-oneway-client1-response.xml'));
		$this->assertEquals($expected, $actual);
		
		// Modifications from client
		$ical = VCalendar::parse(file_get_contents(dirname(__FILE__).'/_syncml/syncml-014-oneway-client2-memento.ics'));
		$collection = _SyncableUnittestCollection::instance();
		$collection->_contents[] = new _SyncableUnittestInstance('_syncml._unit_test2', $ical);
		
		$message = new synd_syncml_message(file_get_contents(dirname(__FILE__).'/_syncml/syncml-014-oneway-client2-modifications.xml'));
		$actual = $message->getResponse();

		$expected = new synd_syncml_message(file_get_contents(dirname(__FILE__).'/_syncml/syncml-014-oneway-client2-response.xml'));
		$this->assertEquals($expected, $actual);
		
		$persistent = $message->getPersistentSession();
		$this->assertEquals('20050102T142005Z', $persistent->getLastAnchor($collection));

		$message->delete();
	}

	function testMaxMsgSize() {
		require_once 'core/lib/calendar/VCalendar.class.inc';
		
		// Session setup (Initialization message)
		$message = new synd_syncml_message(file_get_contents(dirname(__FILE__).'/_syncml/syncml-017-maxmsgsize-init.xml'));
		$actual = $message->getResponse();
		
		$session = $message->getSession();
		$this->assertEquals(2000, $session->getMaxMsgSize());
		
		// Set up collection with two entries
		$ical = VCalendar::parse(file_get_contents(dirname(__FILE__).'/_syncml/syncml-017-maxmsgsize-memento.ics'));
		$collection = _SyncableUnittestCollection::instance();
		$collection->_contents[] = new _SyncableUnittestInstance('_syncml._unit_test1', $ical);
		$collection->_contents[] = new _SyncableUnittestInstance('_syncml._unit_test2', $ical);
		
		// First message in package to to client
		$message = new synd_syncml_message(file_get_contents(dirname(__FILE__).'/_syncml/syncml-017-maxmsgsize-request1.xml'));
		$actual = $message->getResponse();
		
		$expected = new synd_syncml_message(file_get_contents(dirname(__FILE__).'/_syncml/syncml-017-maxmsgsize-response1.xml'));
		$this->assertEquals($expected, $actual);
		//$this->_diff($expected->toString(), $actual->toString());

		// Second and final message in package to to client
		$message = new synd_syncml_message(file_get_contents(dirname(__FILE__).'/_syncml/syncml-017-maxmsgsize-request2.xml'));
		$actual = $message->getResponse();
		
		$expected = new synd_syncml_message(file_get_contents(dirname(__FILE__).'/_syncml/syncml-017-maxmsgsize-response2.xml'));
		$this->assertEquals($expected, $actual);
		//$this->_diff($expected->toString(), $actual->toString());

		$message->delete();
	}

//	function testRefreshClient() {
//		require_once 'core/lib/calendar/VCalendar.class.inc';
//		$collection = _SyncableUnittestCollection::instance();
//		
//		// Session setup (Initialization message)
//		$message = new synd_syncml_message(file_get_contents(dirname(__FILE__).'/_syncml/syncml-015-refresh-client1-init.xml'));
//		$actual = $message->getResponse();
//
//		$expected = new synd_syncml_message(file_get_contents(dirname(__FILE__).'/_syncml/syncml-015-refresh-client1-response.xml'));
//		$this->assertEquals($expected, $actual);
//		//$this->_diff($expected, $actual);
//		
//		// Setup the collection with two existing items
//		$ical = VCalendar::parse(file_get_contents(dirname(__FILE__).'/_syncml/syncml-015-refresh-client2-memento.ics'));
//		$collection = _SyncableUnittestCollection::instance();
//		
//		$item1 = new _SyncableUnittestInstance('_syncml._unit_test1', $ical);
//		$item2 = new _SyncableUnittestInstance('_syncml._unit_test2', $ical);
//		$collection->_contents[$item1->id()] = $item1;
//		$collection->_contents[$item2->id()] = $item2;
//		
//		$persistent = $message->getPersistentSession();
//		$persistent->setGlobalId($collection, '1', '_syncml._unit_test1');
//		$persistent->setGlobalId($collection, '2', '_syncml._unit_test2');
//		$this->assertEquals('1', $persistent->getLocalId($collection, '_syncml._unit_test1'));
//		$this->assertEquals('2', $persistent->getLocalId($collection, '_syncml._unit_test2'));
//
//		// Perform sync
//		$message = new synd_syncml_message(file_get_contents(dirname(__FILE__).'/_syncml/syncml-015-refresh-client2-modifications.xml'));
//		$actual = $message->getResponse();
//
//		$expected = new synd_syncml_message(file_get_contents(dirname(__FILE__).'/_syncml/syncml-015-refresh-client2-response.xml'));
//		$this->assertEquals($expected, $actual);
//
//		// Memento 1 should be updated, memento 2 should be deleted		
//		$contents = $collection->getContents();
//		$this->assertEquals(array($item1->id() => $item1), $contents);
//		$this->assertEquals('1', $persistent->getLocalId($collection, '_syncml._unit_test1'));
//		$this->assertNull($persistent->getLocalId($collection, '_syncml._unit_test2'));
//
//		//debug($expected->toString());
//		//debug($actual->toString());
//		$this->_diff($expected, $actual);
//
//		$message->delete();
//	}
}

