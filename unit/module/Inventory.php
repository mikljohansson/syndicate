<?php
require_once 'PHPUnit2/Framework/TestCase.php';

class _modules_Inventory extends PHPUnit2_Framework_TestCase {
	function setUp() {
		require_once 'core/lib/rpc/RpcTransport.class.inc';
		require_once 'core/model/SyndXMLNode.class.inc';
	}
	
	function tearDown() {
		global $synd_maindb;
		$sql = "
			DELETE FROM synd_inv_lease
			WHERE client_node_id = 'case._unit_test'";
		$this->assertFalse(SyndLib::isError($synd_maindb->query($sql)));

		$sql = "
			DELETE FROM synd_instance
			WHERE node_id IN (
				SELECT i.node_id FROM synd_inv_item i
				WHERE i.owner_node_id = 'case._unit_test')";
		$this->assertFalse(SyndLib::isError($synd_maindb->query($sql)));
	}
	
	function getActivator() {
		return new RpcModuleActivator(Module::getInstance('rpc'), 'php');
	}

	/**
	 * @link	http://www.yoyodesign.org/doc/dtd/alt_wdg_xml.html.en	Custom DTD validator 
	 */
	function testBatchedDevice() {
		$storage = SyndNodeLib::getDefaultStorage('computer');
		$database = $storage->getDatabase();
		
		$sql = "
			DELETE FROM synd_inv_computer
			WHERE info_mb_serial = '123456789'";
		$database->query($sql);
		
		$sql = "
			DELETE FROM synd_inv_interface
			WHERE info_ip_address IN ('192.168.0.1','192.168.1.1')";
		$database->query($sql);

		$xml = file_get_contents(dirname(__FILE__).'/_inventory/device-001.xml');
		$module = Module::getInstance('inventory');
		$node = $module->agent($xml);
		$node->flush();
		
		$this->assertEquals('soap', $node->data['INFO_REMOTE_METHOD']);
		$this->assertEquals('http://192.168.0.1/agent', $node->data['INFO_REMOTE_URI']);
		$this->assertEquals('http://schemas.microsoft.com/clr/nsassem/Inventory.RemoteService/InventoryService#{method}', $node->data['INFO_REMOTE_ACTION']);

		$this->assertEquals('Intel', $node->data['INFO_MB_MAKE']);
		$this->assertEquals('SWV20.86B.0501.P05.0211211659', $node->data['INFO_MB_BIOS']);
		$this->assertEquals('2048', $node->data['INFO_RAM']);
		$this->assertEquals('Intel(R) Xeon(TM) CPU 2.40GHz', $node->data['INFO_CPU_DESC']);
		$this->assertEquals('2400', $node->data['INFO_CPU_CLOCK']);
		$this->assertEquals('2', $node->data['INFO_CPU_COUNT']);
		$this->assertEquals('Enhanced (101- or 102-key)', $node->data['INFO_KEYBOARD']);

		$hdds = array(
			array('INFO_DEVICE' => 'hda', 'INFO_DESC' => 'ST3120026A', 'INFO_SIZE' => 38538, 'INFO_CACHE' => 8192),
			array('INFO_DEVICE' => 'hdc', 'INFO_DESC' => 'ST3120026A', 'INFO_SIZE' => 38538, 'INFO_CACHE' => 8192),
			);
		$this->assertEquals($hdds, $this->_strip($node->getDiskDrives()));

		$roms = array(
			array('INFO_DESC' => 'HL-DT-ST RW/DVD GCC-4480B', 'INFO_MOUNTPOINT' => '/mnt/cdrom'),
			);
		$this->assertEquals($roms, $this->_strip($node->getROMDrives()));

		$soundcards = array(
			array('INFO_DESC' => 'SoundMAX Integrated Digital Audio'),
			);
		$this->assertEquals($soundcards, $this->_strip($node->getSoundDevices()));

		// Test network interfaces
		$actual = array_values(SyndLib::invoke($node->getNetworkInterfaces(), 'getMacAddress'));
		$expected = array(
			'00:07:E9:AB:CD:EF',
			'00:07:E9:BB:CC:DD',);
		$this->assertEquals($expected, $actual);

		$actual = array_values(SyndLib::invoke($node->getNetworkInterfaces(), 'getDescription'));
		$expected = array(
			'Intel Corporation 82546EB Gigabit Ethernet Controller (Copper) (rev 01)',
			'Intel Corporation 82546EB Gigabit Ethernet Controller (Copper) (rev 01)',);
		$this->assertEquals($expected, $actual);
		
		// Test interfaces
		$config = $node->getConfig();
		$interfaces = $config->getInterfaces();
		$this->assertEquals(2, count($interfaces));
		
		if (2 == count($interfaces)) {
			$eth0 = array_shift($interfaces);
			$nic0 = $eth0->getNetworkInterface();
			$this->assertEquals('00:07:E9:AB:CD:EF', $nic0->getMacAddress());
			$this->assertEquals('192.168.0.1', $eth0->getIpAddress());

			$eth1 = array_shift($interfaces);
			$nic1 = $eth1->getNetworkInterface();
			$this->assertEquals('00:07:E9:BB:CC:DD', $nic1->getMacAddress());
			$this->assertEquals('192.168.1.1', $eth1->getIpAddress());
		}
		
		$videocards = array(
			array(
				'INFO_DESC' => 'ATI Technologies Inc Rage XL (rev 27)', 'INFO_RAM' => 8, 'INFO_HRES' => 1280, 'INFO_VRES' => 1024, 'INFO_BITS' => 32, 'INFO_FREQ' => 70),
			);
		$this->assertEquals($videocards, $this->_strip($node->getVideoControllers()));
		
		$monitors = array(
			array('INFO_DESC' => 'Dell', 'INFO_VENDOR_ID' => '2001FP', 'INFO_SIZE' => 20),
			);
		$this->assertEquals($monitors, $this->_strip($node->getMonitors()));
		
		// Test operating system
		$oses = $node->getOperatingSystems();
		$this->assertEquals(1, count($oses));
		
		if (1 == count($oses)) {
			$os = $oses[key($oses)];
			
			$this->assertEquals('Linux', $os->data['INFO_IDENTIFIER']);
			$this->assertEquals('Red Hat Enterprise Linux AS release 3 (Taroon Update 5)', $os->data['INFO_RELEASE']);
			$this->assertEquals('Linux 2.6.13.2 #3 SMP Mon Sep 26 10:13:38 CEST 2005', $os->data['INFO_VERSION']);
			$this->assertEquals('net://global/hdimages/linux/rhentws3_008.base', $os->data['INFO_LOADED_IMAGE']);
			$this->assertEquals('host.example.com', $os->data['INFO_MACHINE_NAME']);
			
			$expected = array(
				array('OS_NODE_ID' => $os->nodeId, 'INFO_PRODUCT' => 'gcc', 'INFO_VERSION' => '3.2.3-49', 'INFO_STATE' => 'ok'),
				array('OS_NODE_ID' => $os->nodeId, 'INFO_PRODUCT' => 'glibc', 'INFO_VERSION' => '2.3.2-95.30', 'INFO_STATE' => 'ok'),
				array('OS_NODE_ID' => $os->nodeId, 'INFO_PRODUCT' => 'procmail', 'INFO_VERSION' => '3.22-9', 'INFO_STATE' => 'ok'),
				
				);
			$actual = $os->getSoftware();
			$this->assertEquals($expected, $actual);
		}
		
		
		$config = $node->getConfig();
		$config->delete();
		
		$node->delete();
	}

	function _strip($values) {
		foreach (array_keys($values) as $key)
			unset($values[$key]['TS_UPDATE']);
		return array_values($values);
	}
	
	function testExpiringLeases() {
		$xmlrpc = Module::getInstance('xmlrpc');
		$inventory = Module::getInstance('inventory');
		
		$lease = SyndNodeLib::factory('lease');
		$lease->setParent($lease->_storage->factory('folder'));
		$lease->data['CLIENT_NODE_ID'] = 'case._unit_test';
		
		$expire = strtotime('2004-01-30');
		$lease->setExpire($expire);
		$this->assertEquals($expire, $lease->getExpire());
		
		$lease->save();
		$lease->flush();

		$remote = Activator::getInstance($this->getActivator()->getEndpoint($inventory));
		$leases = $remote->getExpiringLeases($expire - 30, $expire + 30);
		
		$this->assertFalse(empty($lease->nodeId));
		$this->assertType('array', $leases);
		$this->assertTrue(in_array($lease->nodeId, SyndLib::collect($leases,'nodeId')));
	}

	function testClientLeases() {
		$xmlrpc = Module::getInstance('xmlrpc');
		$inventory = Module::getInstance('inventory');
		
		$lease = SyndNodeLib::factory('lease');
		$lease->setParent($lease->_storage->factory('folder'));
		$lease->data['CLIENT_NODE_ID'] = 'case._unit_test';
		$lease->save();
		$lease->flush();

		$remote = Activator::getInstance($this->getActivator()->getEndpoint($inventory));
		$leases = $remote->getClientLeases('case._unit_test');
		
		$this->assertFalse(empty($lease->nodeId));
		$this->assertEquals(array($lease->nodeId => $lease->nodeId), SyndLib::collect($leases,'nodeId'));
	}
	
	function testConfiguration() {
		$storage = SyndNodeLib::getDefaultStorage('computer');
		$database = $storage->getDatabase();
		
		$sql = "DELETE FROM synd_inv_vlan WHERE info_head = '_unit_test'";
		$database->query($sql);

		$sql = "DELETE FROM synd_instance WHERE node_id IN (SELECT node_id FROM synd_inv_item WHERE owner_node_id = 'case._unit_test')";
		$database->query($sql);

		$sql = "DELETE FROM synd_instance WHERE node_id IN (SELECT node_id FROM synd_inv_folder WHERE create_node_id = 'null._unit_test')";
		$database->query($sql);

		$sql = "DELETE FROM synd_class WHERE name = '_unit_test: This class is safe to delete'";
		$database->query($sql);

		$class = SyndNodeLib::factory('class');
		$class->data['CREATE_NODE_ID'] = 'null._unit_test';
		$class->data['NAME'] = '_unit_test: This class is safe to delete';
		$class->addField('tftp-server-name', 'DhcpText');
		$class->addField('synd.inventory-server-name', 'DhcpIpAddress');
		$class->addField('StartPage', 'RemboField');
		$class->addField('Options', 'RemboOptions');
		$class->save();
		
		$folder = SyndNodeLib::factory('folder');
		$folder->setClass($class);
		$folder->data['INFO_HEAD'] = '_unit_test1';
		$folder->data['CREATE_NODE_ID'] = 'null._unit_test';
		$folder->data['tftp-server-name'] = 'tftp.example.com';
		$folder->data['synd.inventory-server-name'] = '1.2.3.4';
		$folder->data['StartPage'] = 'cache://global/example.shtml';
		$folder->data['Options'] = 'autoboot admin';
		$folder->save();

		$folder2 = SyndNodeLib::factory('folder');
		$folder2->data['INFO_HEAD'] = '_unit_test2';
		$folder2->data['CREATE_NODE_ID'] = 'null._unit_test';
		$folder2->save();
		
		$node = $folder->appendChild(SyndNodeLib::factory('computer'));
		$node->data['OWNER_NODE_ID'] = 'case._unit_test';
		$node->data['INFO_MAKE'] = '_unit_test: This item is safe to delete';
		$node->save();
		
		$node2 = $folder2->appendChild(SyndNodeLib::factory('computer'));
		$node2->data['OWNER_NODE_ID'] = 'case._unit_test';
		$node2->data['INFO_MAKE'] = '_unit_test: This item is safe to delete';
		$node2->save();

		$nic = $node->appendChild($node->_storage->factory('nic'));
		$nic->setMacAddress('01:02:03:04:05:06');
		$nic->save();

		$nic2 = $node->appendChild($node->_storage->factory('nic'));
		$nic2->setMacAddress('01:02:03:04:05:07');
		$nic2->save();
		
		$nic3 = $node2->appendChild($node2->_storage->factory('nic'));
		$nic3->setMacAddress('01:02:03:04:05:08');
		$nic3->save();
		
		$config = $node->getConfig();
		$config->save();
		
		$config2 = $node2->getConfig();
		$config2->save();

		$interface = $config->appendChild($config->_storage->factory('interface'));
		$interface->setHostname('host1.example.com');
		$interface->setIpAddress('1.2.3.4');
		$interface->setNetworkInterface($nic);
		$interface->save();
		
		$interface2 = $config->appendChild($config->_storage->factory('interface'));
		$interface2->setHostname('host2.example.com');
		$interface2->setIpAddress('1.2.3.5');
		$interface2->setNetworkInterface($nic2);
		$interface2->save();

		$interface3 = $config2->appendChild($config2->_storage->factory('interface'));
		$interface3->setHostname('host3.example.com');
		$interface3->setIpAddress('1.2.3.6');
		$interface3->setNetworkInterface($nic3);
		$interface3->save();

		$vlan = $folder->appendChild($node->_storage->factory('vlan'));
		$vlan->data['INFO_HEAD'] = '_unit_test';
		$vlan->addNetwork('1.2.3.0', '255.255.255.0');
		$vlan->save();

		$node->flush();
		
		// Flush buffers to browser to header() won't succeed to affect unittest rendering
		flush();

		$inventory = Module::getInstance('inventory');
		$_REQUEST['network'] = array('1.2.3.0:16', '1.2.4.0:24');

		// Test dhcpd.conf generation
		$inventory->loadPlugin('dhcp');
		$result = $inventory->request(new HttpRequest($_SESSION, '/', 'dhcp/dhcpd.conf'));
		$actual = $result['content'];
		
		$page = new Template();
		$page->assign('uri', 'http://'.$_SERVER['SERVER_NAME'].$_SERVER['REQUEST_URI']);
		$expected = $page->fetch(dirname(__FILE__).'/_inventory/dhcpd.conf');
		$this->assertEquals($expected, $actual);

		// Test vmps.db generation
		$inventory->loadPlugin('vmps');
		$result = $inventory->request(new HttpRequest($_SESSION, '/', 'vmps/vmps.db'));;
		$actual = $result['content'];
		
		$page = new Template();
		$expected = $page->fetch(dirname(__FILE__).'/_inventory/vmps.db');
		$this->assertEquals($expected, $actual);

		// Test rembo.conf generation
		$inventory->loadPlugin('rembo');
		$result = $inventory->request(new HttpRequest($_SESSION, '/', 'rembo/rembo.conf'));
		$actual = $result['content'];
		
		$page = new Template();
		$page->assign('uri', 'http://'.$_SERVER['SERVER_NAME'].$_SERVER['REQUEST_URI']);
		$expected = $page->fetch(dirname(__FILE__).'/_inventory/rembo.conf');
		$this->assertEquals($expected, $actual);

		$vlan->delete();
		$node->delete();
		$node2->delete();
		$folder->delete();
		$class->delete();
	}

	/**
	 * Execute afterwards to get the DHCP codes right
	 */
	function testTypes() {
		require_once 'core/model/node/class.class.inc';
		$inventory = Module::getInstance('inventory');
		$inventory->loadPlugin('dhcp');
		$inventory->loadPlugin('rembo');
		
		$type = synd_node_class::type('DhcpText', 'synd.unit-test-field');
		$this->assertEquals('synd.unit-test-field', $type->toString());
		$this->assertTrue($type->isInheritedFrom('DhcpDatatype'));
		$this->assertEquals('synd', $type->getOptionSpace());
		$this->assertEquals(2, $type->getOptionCode());
		$this->assertFalse(true === $type->validate($value = ''));
		
		$type = synd_node_class::type('DhcpUnsignedInteger32', 'synd.unit-test-field2');
		$this->assertTrue($type->isInheritedFrom('DhcpDatatype'));
		$this->assertEquals('synd', $type->getOptionSpace());
		$this->assertEquals(3, $type->getOptionCode());
		$this->assertTrue(true === $type->validate($value = '123'));
		$this->assertFalse(true === $type->validate($value = '12A'));
		$this->assertFalse(true === $type->validate($value = ''));

		$type = synd_node_class::type('DhcpIpAddress', 'unit-test-field');
		$this->assertTrue($type->isInheritedFrom('DhcpDatatype'));
		$this->assertEquals(0, $type->getOptionSpace());
		$this->assertTrue(true === $type->validate($value = '1.2.3.4'));
		$this->assertFalse(true === $type->validate($value = '1.2.3.A'));
		$this->assertFalse(true === $type->validate($value = '1.2.3.1234'));
		$this->assertFalse(true === $type->validate($value = ''));

		$type = synd_node_class::type('RemboField', 'unit-test-field');
		$this->assertTrue($type->isInheritedFrom('RemboField'));
		$this->assertTrue(true === $type->validate($value = 'foo bar'));
		$this->assertFalse(true === $type->validate($value = "foo \r\nbar"));
		$this->assertFalse(true === $type->validate($value = "foo \0bar"));
		$this->assertFalse(true === $type->validate($value = ''));

		$type = synd_node_class::type('RemboOptions', 'unit-test-field');
		$this->assertTrue($type->isInheritedFrom('RemboField'));
		$this->assertTrue(true === $type->validate($value = 'foo bar'));
		$this->assertFalse(true === $type->validate($value = 'foo$'));
		$this->assertFalse(true === $type->validate($value = ''));
	}
}
