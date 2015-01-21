<?php
require_once 'PHPUnit2/Framework/TestCase.php';
require_once 'core/lib/crypto/Keyserver.class.inc';

class _lib_Pgp extends PHPUnit2_Framework_TestCase {
	function setUp() {
		require_once 'core/lib/crypto/driver/pgp.class.inc';
		require_once 'core/lib/Mime.class.inc';
	}
	
	function testKey() {
		$pgp = synd_crypto_pgp::factory();
		$key = $pgp->key(file_get_contents(dirname(__FILE__).'/_pgp/mikl.asc'));

		$expected = array(strtr('Mikael Johansson <mikael SPAMMENOT synd DOT info>', array(' SPAMMENOT ' => '@', ' DOT ' => '.')));
		$actual = SyndLib::invoke($key->getIdentities(), 'toString');
		$this->assertEquals($expected, $actual);
		
		$expected = array(strtr('mikael SPAMMENOT synd DOT info', array(' SPAMMENOT ' => '@', ' DOT ' => '.')));
		$actual = SyndLib::invoke($key->getIdentities(), 'getEmail');
		$this->assertEquals($expected, $actual);
		
		$expected = '0xDD60C375';
		$actual = $key->getKeyid();
		$this->assertEquals($expected, $actual);
		
		$expected = 'B9DCDFE044C2467DD073B72D87085750DD60C375';
		$actual = $key->getFingerprint();
		$this->assertEquals($expected, $actual);

		$this->assertFalse($key->isSigningKey());
		$this->assertTrue($key->isVerificationKey());
		$this->assertTrue($key->isEncryptionKey());
		$this->assertFalse($key->isDecryptionKey());
	}
	
	function testVerify() {
		$pgp = synd_crypto_pgp::factory();
		$keyserver = new _lib_Pgp_Keyserver(file_get_contents(dirname(__FILE__).'/_pgp/mikl.asc'));

		$mime = Mime::parse(file_get_contents(dirname(__FILE__).'/_pgp/pgp-001-signed-mime.msg'), array($this, '_callback_verbatim'));
		$actual = $pgp->verify($keyserver, $mime, $key, $keyid);
		$this->assertType('MimeTextpart', $actual);
		
		$expected = "Test sign using PGP/MIME";
		$actual = $actual->getContent();
		$this->assertEquals($expected, $actual);

		$mime = Mime::parse(file_get_contents(dirname(__FILE__).'/_pgp/pgp-002-invalid-signed-mime.msg'), array($this, '_callback_verbatim'));
		$actual = $pgp->verify($keyserver, $mime, $key, $keyid);
		$this->assertFalse($actual);

		$mime = Mime::parse(file_get_contents(dirname(__FILE__).'/_pgp/pgp-003-signed-clear.msg'), array($this, '_callback_verbatim'));
		$actual = $pgp->verify($keyserver, $mime, $key, $keyid);
		$this->assertType('MimeTextpart', $actual);
		
		$expected = "Test sign using clearsign\r\n";
		$actual = $actual->getContent();
		$this->assertEquals($expected, $actual);

		$mime = Mime::parse(file_get_contents(dirname(__FILE__).'/_pgp/pgp-004-invalid-signed-clear.msg'), array($this, '_callback_verbatim'));
		$actual = $pgp->verify($keyserver, $mime, $key, $keyid);
		$this->assertFalse($actual);
	}
	
	function testKeyserver() {
		$keyserver = new synd_crypto_pgp_hpks('http://subkeys.pgp.net:11371');
		$driver = synd_crypto_pgp::factory();
		
		$expected = $driver->key(file_get_contents(dirname(__FILE__).'/_pgp/mikl.asc'));
		$actual = $keyserver->fetch($driver, '0xDD60C375');
		
		$this->assertEquals($expected->getKeyid(), $actual->getKeyid());
		$this->assertEquals($expected->getIdentity()->toString(), $actual->getIdentity()->toString());
	}
	
	function _callback_verbatim() {
		return true;
	}
}

class _lib_Pgp_Keyserver implements Keyserver {
	private $_key = null;
	
	function __construct($key) {
		$this->_key = $key;
	}
	
	function fetch(CryptoProtocolFactory $factory, $keyid) {
		return $factory->getDriver('pgp')->key($this->_key);
	}

	function store(CryptoKey $key) {}
	function delete(CryptoKey $key) {}

	function search(CryptoProtocolFactory $factory, $query = null, $offset = 0, $limit = 50) {}
}
