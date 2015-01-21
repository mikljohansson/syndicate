<?php
require_once 'PHPUnit2/Framework/TestCase.php';

class _plugin_Vmps extends PHPUnit2_Framework_TestCase {
	var $_plugin = null;

	function setUp() {
		$inventory = Module::getInstance('inventory');
		$this->_plugin = $inventory->loadPlugin('vmps');
	}

	function testLogging() {
		$computer = SyndNodeLib::factory('computer');
		$computer->save();

		$sql = "
			DELETE FROM synd_inv_nic
			WHERE 
				info_mac_address = '01:02:03:04:05:06' OR 
				info_mac_address = '01:02:03:04:05:07'";
		$result = $computer->_db->query($sql);
		$this->assertFalse(SyndLib::isError($result));

		$nic1 = $computer->appendChild($computer->_storage->factory('nic'));
		$nic1->setMacAddress('01:02:03:04:05:06');
		$nic1->save();

		$nic2 = $computer->appendChild($computer->_storage->factory('nic'));
		$nic2->setMacAddress('01:02:03:04:05:07');
		$nic2->save();
		$computer->flush();
		
		$log = file_get_contents(dirname(__FILE__).'/_vmps/vmps-001.log');
		$this->_plugin->log($log);

		$persistent = $nic1->_storage->getPersistentStorage();
		$copy1 = $persistent->getInstance($nic1->nodeId);
		$copy2 = $persistent->getInstance($nic2->nodeId);
		
		$this->assertEquals(strtotime("Feb 5".date(" Y ")."04:17:00"), $copy1->data['INFO_LAST_SEEN']);
		$this->assertEquals("192.168.0.2", $copy1->data['INFO_LAST_SWITCH']);
		$this->assertEquals("Fa0/8", $copy1->data['INFO_LAST_SWITCH_PORT']);
		$vlan = $copy1->getLastVirtualLan();
		$this->assertEquals("UnitTest", $vlan->toString());
		
		$this->assertEquals(strtotime("Feb 5".date(" Y ")."04:18:00"), $copy2->data['INFO_LAST_SEEN']);
		$this->assertEquals("192.168.0.3", $copy2->data['INFO_LAST_SWITCH']);
		$this->assertEquals("Fa0/9", $copy2->data['INFO_LAST_SWITCH_PORT']);
		$vlan = $copy2->getLastVirtualLan();
		$this->assertEquals("UnitTest", $vlan->toString());

		$computer->delete();
		$vlan->delete();
	}
}
