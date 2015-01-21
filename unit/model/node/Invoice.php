<?php
require_once 'unit/SyndNodeTestCase.class.inc';

class _model_Node_Invoice extends SyndNodeTestCase {
	function testInvoiceNumber() {
		$storage = SyndNodeLib::getDefaultStorage('invoice');
		$persistent = $storage->getPersistentStorage();
		$database = $persistent->getDatabase();
		
		$client = $storage->factory('unit_test');
		$client->setSocialSecurityNumber('1234561234');

		$semester = date('n') < 8 ? 1 : 2;
		$ssnPart = substr($client->getSocialSecurityNumber(),0,6).substr(date('y'),-1).$semester;
		$unique = $database->nextId('synd_issue_invoice') % $ssnPart % 10;
		
		$unique++;
		$unique %= 10;
		
		$number = $ssnPart.$unique.((strlen($ssnPart)+strlen($unique)+2) % 10);
		$expected = $number.synd_node_invoice::_parity($number);

		$invoice = $storage->factory('invoice');
		$invoice->setCustomer($client);
		$this->assertEquals($expected, $invoice->getInvoiceNumber());
		
		// Test next invoice number
		$unique++;
		$unique %= 10;

		$number = $ssnPart.$unique.((strlen($ssnPart)+strlen($unique)+2) % 10);
		$expected = $number.synd_node_invoice::_parity($number);

		$invoice = SyndNodeLib::factory('invoice');
		$invoice->setCustomer($client);
		$this->assertEquals($expected, $invoice->getInvoiceNumber());
	}
	
	function testParity() {
		SyndNodeLib::loadClass('invoice');
		$number = '800814123';
		$parity = synd_node_invoice::_parity($number);
		$this->assertEquals('9', $parity);
	}
}
