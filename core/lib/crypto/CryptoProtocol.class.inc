<?php
require_once 'core/lib/crypto/Keyserver.class.inc';
require_once 'core/lib/Mime.class.inc';

/**
 * Instantiates cryptography protocol drivers
 * @access		public
 * @package		synd.core.lib
 */
interface CryptoProtocolFactory {
	/**
	 * Instantiates a driver given the protocol
	 * @param	string				Protocol to support (ie. "pgp")
	 * @return	CryptoProtocol
	 */
	function getDriver($protocol);
}

/**
 * Cryptography protocol driver interface
 * @access		public
 * @package		synd.core.lib
 */
interface CryptoProtocol {
	/**
	 * Creates a new instance of this protocol
	 * @return	CryptoProtocol
	 */
	static function factory();

	/**
	 * Returns the name of this protocol (ie. "pgp")
	 * @return	string
	 */
	function getProtocol();
	
	/**
	 * Parses the string representation if a key
	 * @param	string		String representation of key
	 * @return	CryptoKey
	 */
	function key($buffer);

	/**
	 * Parses multiple string representations of keys
	 * @param	string		String representation of keys
	 * @return	array
	 */
	function keyring($buffer);
	
	/**
	 * Instantiates a keyserver
	 * @param	string		Endpoint uri
	 * @return	Keyserver
	 */
	function keyserver($uri);

	/**
	 * Signs a message
	 * @throws	RuntimeException
	 * @param	Keyserver	Keyserver to ask for keys
	 * @param	MimePart	The message to sign
	 * @return	MimePart	Returns the signed message, false on failure
	 */
	function sign(Keyserver $keyserver, MimePart $message);
	
	/**
	 * Verifies a signed message
	 * @throws	RuntimeException
	 * @param	Keyserver	Keyserver to ask for keys
	 * @param	MimePart	The signed message
	 * @param	CryptoKey	Fetched key will be writted to this variable
	 * @param	string		Keyid of detected key will be written to this variable
	 * @return	MimePart	Returns the cleartext message, false on failure
	 */
	function verify(Keyserver $keyserver, MimePart $message, &$key, &$keyid);
	
	/**
	 * Encrypts a message
	 * @throws	RuntimeException
	 * @param	Keyserver	Keyserver to ask for keys
	 * @param	MimePart	The message to encrypt
	 * @return	MimePart	Returns the encrypted message, false on failure
	 */
	function encrypt(Keyserver $keyserver, MimePart $message);

	/**
	 * Decrypts a message
	 * @throws	RuntimeException
	 * @param	Keyserver	Keyserver to ask for keys
	 * @param	MimePart	The encrypted message
	 * @return	MimePart	Returns the cleartext message, false on failure
	 */
	function decrypt(Keyserver $keyserver, MimePart $message);
}

/**
 * Cryptography key interface
 * @access		public
 * @package		synd.core.lib
 */
interface CryptoKey {
	/**
	 * Returns the string representation of this key
	 * @return	string
	 */
	function toString();
	
	/**
	 * @return	string
	 */
	function getKeyid();
	
	/**
	 * @return	string
	 */
	function getFingerprint();

	/**
	 * Returns the created unix timestamp
	 * @return	integer	Returns null if no timestamp set
	 */
	function getCreated();

	/**
	 * Returns the expiry unix timestamp
	 * @return	integer	Returns null if no timestamp set
	 */
	function getExpiry();

	/**
	 * Returns the primary identity
	 * @return	CryptoIdentity
	 */
	function getIdentity();
	
	/**
	 * Returns the identities who this key is valid for
	 * @return	array
	 */
	function getIdentities();

	/**
	 * Key trust level
	 * @return	int
	 */
	function getTrust();
	
	/**
	 * Specify key trust level
	 * @param	int	Trust level
	 */
	function setTrust($trust);

	/**
	 * Key protocol
	 * @return	int
	 */
	function getProtocol();

	/**
	 * Key is revoked
	 * @return	bool
	 */
	function isRevoked();

	/**
	 * Key may be used for signing (eg. private key)
	 * @return	bool
	 */
	function isSigningKey();

	/**
	 * Key may be used for verifying signatures (eg. public key)
	 * @return	bool
	 */
	function isVerificationKey();

	/**
	 * Key may be used for encryption (eg. public key)
	 * @return	bool
	 */
	function isEncryptionKey();

	/**
	 * Key may be used for decryption (eg. private key)
	 * @return	bool
	 */
	function isDecryptionKey();
}

/**
 * Chain-of-command forwarding key
 */
class ForwardingCryptoKey implements CryptoKey {
	protected $_next = null;
	
	function __construct(CryptoKey $next) {
		$this->_next = $next;
	}

	function toString()				{return $this->_next->toString();}
	function getKeyid()				{return $this->_next->getKeyid();}
	function getKeyids()			{return $this->_next->getKeyids();}
	function getFingerprint()		{return $this->_next->getFingerprint();}
	function getCreated()			{return $this->_next->getCreated();}
	function getExpiry()			{return $this->_next->getExpiry();}
	function getIdentity()			{return $this->_next->getIdentity();}
	function getIdentities()		{return $this->_next->getIdentities();}
	function getTrust()				{return $this->_next->getTrust();}
	function setTrust($trust)		{return $this->_next->setTrust($trust);}
	function getProtocol()			{return $this->_next->getProtocol();}
	function isRevoked()			{return $this->_next->isRevoked();}
	function isSigningKey()			{return $this->_next->isSigningKey();}
	function isVerificationKey()	{return $this->_next->isVerificationKey();}
	function isEncryptionKey()		{return $this->_next->isEncryptionKey();}
	function isDecryptionKey()		{return $this->_next->isDecryptionKey();}
}

/**
 * Cryptography identity interface
 * @access		public
 * @package		synd.core.lib
 */
interface CryptoIdentity {
	/**
	 * Returns a human readable string (ie. "Mikael Johansson <mikl@example.com>")
	 * @return	string
	 */
	function toString();

	/**
	 * Returns the name of this identity
	 * @return	string
	 */
	function getName();

	/**
	 * Returns the email of this identity
	 * @return	string
	 */
	function getEmail();
}

/**
 * Simple default identity implementation
 * @package		synd.core.lib
 */
class SimpleCryptoIdentity implements CryptoIdentity {
	private $_name = null;
	private $_email = null;
	
	function __construct($name, $email) {
		$this->_name = $name;
		$this->_email = $email;
	}
	
	function toString() {
		return "{$this->_name} <{$this->_email}>";
	}
	
	function getName() {
		return $this->_name;
	}
	
	function getEmail() {
		return $this->_email;
	}
}
