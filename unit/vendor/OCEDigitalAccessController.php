<?php
require_once 'PHPUnit2/Framework/TestCase.php';

class _vendor_OCEDigitalAccessController extends PHPUnit2_Framework_TestCase {
	var $_oce = null;
	
	function setUp() {
		require_once 'oce_printer.inc';
		$this->_oce = connect_oce_printer();
	}
	
	function tearDown() {
		$this->_oce->disconnect();
	}
	
	function testAccounts() {
		$accounts = $this->_oce->getAccounts();
		
		$expected = array(
			'Account' => '99997',
			'Status' => '+',
			'Username' => 'Key Operator',
			);
		$account = array_shift($accounts);

		$this->assertNotNull($account);
		if (null == $account) return;

		$this->assertEquals($expected, SyndLib::array_kintersect($expected, $account));
	}
	
	function testCreateAccount() {
		$account = $this->_oce->addAccount('Unit Test');
		$this->assertNotNull($account);
		if (null == $account) return;
		$id = $account['Account'];

		$account = $this->_oce->getAccount($id);
		$this->assertNotNull($account);
		if (null == $account) return;
		$this->assertEquals('Unit Test', $account['Username']);
		
		$this->_oce->setCounters($account['Account'], 1, 2, 3, 4);
		$account = $this->_oce->getAccount($account['Account']);
		$this->assertEquals(1, $account['Copies']);
		$this->assertEquals(2, $account['Print']);
		$this->assertEquals(3, $account['Scan']);
		$this->assertEquals(4, $account['Limit']);
		
		$this->_oce->disconnect();
		$this->_oce = connect_oce_printer();
		
		$account = $this->_oce->getAccount($id);
		$this->assertNotNull($account);
		if (null == $account) return;
		$this->assertEquals('Unit Test', $account['Username']);

		$account = $this->_oce->getAccount($account['Account']);
		$this->assertEquals(1, $account['Copies']);
		$this->assertEquals(2, $account['Print']);
		$this->assertEquals(3, $account['Scan']);
		$this->assertEquals(4, $account['Limit']);
		
		$this->_oce->delAccount($id);
		$this->_oce->disconnect();
		$this->_oce = connect_oce_printer();

		$account = $this->_oce->getAccount($id);
		$this->assertTrue(null == $account || 'Unit Test' != $account['Username']);
	}
}

