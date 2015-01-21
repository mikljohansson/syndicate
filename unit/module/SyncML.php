<?php
require_once 'unit/module/SyncMLTestcase.class.inc';
require_once 'core/model/ISyncable.class.inc';
require_once 'core/model/syncml/message.class.inc';
require_once 'core/model/syncml/source.class.inc';
require_once 'core/model/syncml/target.class.inc';

class _modules_SyncML extends SyncMLTestcase {
	function testEnabled() {
		$syncml = Module::getInstance('syncml');
		$this->assertNotNull($syncml);
	}
	
	function testParse() {
		$session = new SyncMLSession(1);
		$xml = file_get_contents(dirname(__FILE__).'/_syncml/syncml-001-tostring.xml');
		$actual = new synd_syncml_message($xml);
		
		$expected = new synd_syncml_message();
		$syncml = $expected->appendChild(synd_syncml_node::factory('SyncML'));
		
		$header = $syncml->appendChild(synd_syncml_node::factory('SyncHdr'));
		$header->appendChild(new synd_syncml_node('VerDTD', '1.1'));
		$header->appendChild(new synd_syncml_node('VerProto', 'SyncML/1.1'));
		$header->appendChild(new synd_syncml_node('SessionID', (string)$session->getSessionId()));
		$header->appendChild(new synd_syncml_node('MsgID', (string)$session->_id));

		$header->appendChild(new synd_syncml_target('Target', 'http://www.example.com/synd/syncml/'));
		$header->appendChild(new synd_syncml_source('Source', '_unit_test_device', 'foo'));
		
		$meta = $header->appendChild(synd_syncml_node::factory('Meta'));
		$meta->appendChild(new synd_syncml_node('MaxMsgSize', '10000'));
		
		$body = $syncml->appendChild(synd_syncml_node::factory('SyncBody'));
		
		$get = $body->appendChild(synd_syncml_node::factory('Get'));
		$get->appendChild(new synd_syncml_node('CmdID', '2'));

		$meta = $get->appendChild(synd_syncml_node::factory('Meta'));
		$meta->appendChild(new synd_syncml_node('Type', 'application/vnd.syncml-devinf+xml'));
		
		$item = $get->appendChild(synd_syncml_node::factory('Item'));
		$item->appendChild(new synd_syncml_target('Target', './devinf11'));

		$body->appendChild(synd_syncml_node::factory('Final'));
		
		$this->assertEquals($expected, $actual);
	}
	
	function testToString() {
		$message = new synd_syncml_message(file_get_contents(dirname(__FILE__).'/_syncml/syncml-001-tostring.xml'));
		$actual = $message->toString();

		$expected = file_get_contents(dirname(__FILE__).'/_syncml/syncml-002-tostring.xml');

		$this->assertEquals($expected, $actual);
		//$this->_diff($expected, $actual);
	}

	function testToTimestamp() {
		require_once 'core/model/syncml/alert.class.inc';
		$ts = time();
		$this->assertEquals(
			date('Y-m-d H:i:s', $ts), 
			date('Y-m-d H:i:s', synd_syncml_alert_strategy::toTimestamp(gmdate('Ymd\THis\Z', $ts))));
	}

	/**
	 * The client Put's its device info and the server responds with
	 * a status 200 (Ok) to the Put command and responds to the Get command
	 * with a Results containing its own device info
	 */
	function testDeviceInfo() {
		$message = new synd_syncml_message(file_get_contents(dirname(__FILE__).'/_syncml/syncml-003-devinf.xml'));
		$actual = $message->getResponse();
		
		$buffer = str_replace(
			'<DevID>http://www.example.com/synd/syncml/</DevID>', 
			'<DevID>http://'.$_SERVER['SERVER_NAME'].tpl_view('syncml').'</DevID>',
			file_get_contents(dirname(__FILE__).'/_syncml/syncml-003-response.xml'));
		$expected = new synd_syncml_message($buffer);

		$this->assertEquals($expected, $actual);
		//debug($expected->toString());
		//debug($actual->toString());
		//$this->_diff($expected->toString(), $actual->toString());
		
		$actual->delete();
	}

	/**
	 * Checks that the device info sent by client is stored in the session
	 */
	function testDeviceInfoStorage() {
		$message = new synd_syncml_message(file_get_contents(dirname(__FILE__).'/_syncml/syncml-003-devinf.xml'));
		$message->getResponse();
		
		$buffer = str_replace(
			'<DevID>http://www.example.com/synd/syncml/</DevID>', 
			'<DevID>http://'.$_SERVER['SERVER_NAME'].tpl_view('syncml').'</DevID>',
			file_get_contents(dirname(__FILE__).'/_syncml/syncml-003-response.xml'));
		$response = new synd_syncml_message($buffer);
		
		$document = $message->getDocument();
		$expected = $document->selectSingleNode('SyncBody', 'Put', 'Item', 'Data', 'DevInf');
		$this->assertFalse($expected->isNull());
		
		$persistent = $message->getPersistentSession();
		$actual = $persistent->getDeviceInfo();

		$this->assertEquals($expected, $actual);

		$message->delete();
	}

	function testConnection() {
		require_once 'core/lib/rpc/RpcTransport.class.inc';
		$input = file_get_contents(dirname(__FILE__).'/_syncml/syncml-008-connection.xml');
		
		$connection = new HttpConnection('http://'.$_SERVER['SERVER_NAME']);
		$response = $connection->postRequest(tpl_view('syncml'), $input, 'application/vnd.syncml+xml');
		
		$this->assertEquals(200, $connection->getStatus());
		$actual = new synd_syncml_message($response);
		$expected = new synd_syncml_message(file_get_contents(dirname(__FILE__).'/_syncml/syncml-008-response.xml'));

		$this->assertEquals($expected, $actual);
		//$this->_diff($expected, $actual);

		$actual->delete();
	}

	function testConnectionWBXML() {
		require_once 'core/lib/rpc/RpcTransport.class.inc';
		$message = new synd_syncml_message();
		
		$input = file_get_contents(dirname(__FILE__).'/_syncml/syncml-008-connection.xml');
		$input = $message->_encode($input);
		
		$connection = new HttpConnection('http://'.$_SERVER['SERVER_NAME']);
		$response = $connection->postRequest(tpl_view('syncml'), $input, 'application/vnd.syncml+wbxml');
		$response = $message->_decode($response);
		
		$this->assertEquals(200, $connection->getStatus());
		$actual = new synd_syncml_message($response);
		$expected = new synd_syncml_message(file_get_contents(dirname(__FILE__).'/_syncml/syncml-008-response.xml'));

		$this->assertEquals($expected, $actual);
		//$this->_diff($expected, $actual);

		$actual->delete();
	}
	
	/**
	 * Client sends Cred element with auth:basic credentials
	 */
	function testAuthorizationFail() {
		if (null !== ($user = SyndNodeLib::getInstance('user._unit_test'))) {
			$user->delete();
			$user->flush();
		}
		
		// Test authorization denied
		$actualBuf = file_get_contents(dirname(__FILE__).'/_syncml/syncml-006-auth-basic.xml');
		$message = new synd_syncml_message(str_replace('__DATA__', base64_encode('_unit_test:password'), $actualBuf));
		$actual = $message->getResponse();
		
		$buffer = file_get_contents(dirname(__FILE__).'/_syncml/syncml-006-response.xml');
		$expected = new synd_syncml_message(str_replace('__STATUS__', 401, $buffer));
		$this->assertEquals($expected, $actual);
		//$this->_diff($expected->toString(), $actual->toString());

		$message->delete();
	}
	
	/**
	 * Client sends Cred element with auth:basic credentials
	 */
	function testAuthenticateBasic() {
		SyndNodeLib::loadClass('user');
		if (null != ($user = synd_node_user::resolveLogin('_unit_test'))) {
			$user->delete();
			$user->flush();
		}
		
		$user = SyndNodeLib::factory('user');
		$user->setUsername('_unit_test');
		$user->setPassword($password = substr(md5(uniqid('')),0,8));
		$user->save();
		$user->flush();
		
		$actualBuf = file_get_contents(dirname(__FILE__).'/_syncml/syncml-006-auth-basic.xml');
		$message = new synd_syncml_message(str_replace('__DATA__', base64_encode('_unit_test:'.$password), $actualBuf));
		$actual = $message->getResponse();
		
		$buffer = file_get_contents(dirname(__FILE__).'/_syncml/syncml-006-response.xml');
		$expected = new synd_syncml_message(str_replace('__STATUS__', 212, $buffer));
		$this->assertEquals($expected, $actual);

		$session = $message->getSession();
		$actual = $session->getAuthenticatedUser();
		$this->assertEquals($user, $actual);

		$message->delete();
		$user->delete();
	}
	
	function testParseQuotedPrintable() {
		$buffer = file_get_contents(dirname(__FILE__).'/_syncml/syncml-016-quoted-printable.xml');
		$message = new synd_syncml_message($buffer);
		$document = $message->getDocument();
		
		$data = $document->selectSingleNode('SyncBody', 'Sync', 'Replace', 'Item', 'Data');
		$ical = VCalendar::parse($data->getContent());
		$this->assertEquals("None\r\n123", trim($ical->getDescription()));
	}
}
