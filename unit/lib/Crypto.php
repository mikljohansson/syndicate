<?php
require_once 'PHPUnit2/Framework/TestCase.php';
require_once 'core/lib/crypto/CryptoProtocol.class.inc';
require_once 'core/lib/crypto/Keyserver.class.inc';

class _lib_Crypto extends PHPUnit2_Framework_TestCase {
	function setUp() {
		require_once 'core/lib/crypto/CompositeKeyserver.class.inc';
	}

	function testCompositeKeyserver() {
		$key1 = new _lib_Crypto_Key('1');
		$key2 = new _lib_Crypto_Key('2');
		
		$primary = new _lib_Crypto_Keyserver($key1);
		$secondary = new _lib_Crypto_Keyserver($key2);
		$keyserver = new CompositeKeyserver($primary, array($secondary));

		$actual = $primary->fetch(new _lib_Crypto_Protocol(), '1', 0);
		$this->assertEquals($key1, $actual);

		$actual = $keyserver->fetch(new _lib_Crypto_Protocol(), '1', 0);
		$this->assertEquals($key1, $actual);

		$actual = $keyserver->fetch(new _lib_Crypto_Protocol(), '2', 0);
		$this->assertEquals($key2, $actual);
		
		$actual = $primary->fetch(new _lib_Crypto_Protocol(), '2', 0);
		$this->assertEquals($key2, $actual);
	}
}

class _lib_Crypto_Protocol implements CryptoProtocolFactory {
	function getDriver($protocol) {}
}

class _lib_Crypto_Key implements CryptoKey {
	private $_keyid = null;
	
	function __construct($keyid) {
		$this->_keyid = $keyid;
	}
	
	function toString() {}
	
	function getKeyid() {
		return $this->_keyid;
	}
	
	function getFingerprint() {}
	function getCreated() {}
	function getExpiry() {}
	function getIdentity() {}
	function getIdentities() {}
	function getTrust() {}
	function setTrust($trust) {}
	function getProtocol() {}
	function isRevoked() {}
	function isSigningKey() {}
	function isVerificationKey() {}
	function isEncryptionKey() {}
	function isDecryptionKey() {}
}

class _lib_Crypto_Keyserver implements Keyserver {
	private $_key = null;
	
	function __construct($key) {
		$this->_key = $key;
	}
	
	function fetch(CryptoProtocolFactory $factory, $keyid) {
		return $this->_key->getKeyid() == $keyid ? $this->_key : false;
	}
	
	function find(CryptoProtocol $driver, $keyid, $flags) {
		return $this->_key->getKeyid() == $keyid ? $this->_key : false;
	}
	
	function store(CryptoKey $key) {
		$this->_key = $key;
	}
	
	function delete(CryptoKey $key) {
		$this->_key = null;
	}

	function search(CryptoProtocolFactory $factory, $query = null, $offset = 0, $limit = 50) {
		return array($this->_key);
	}
}
