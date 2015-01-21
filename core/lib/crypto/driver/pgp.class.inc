<?php
require_once 'core/lib/crypto/CryptoProtocol.class.inc';
require_once 'core/lib/crypto/Keyserver.class.inc';
require_once 'core/lib/crypto/Pem.class.inc';
require_once 'core/lib/SyndLib.class.inc';
require_once 'core/lib/Mime.class.inc';

/**
 * Abstract PGP protocol factory
 * @access		public
 * @package		synd.core.lib
 */
abstract class synd_crypto_pgp implements CryptoProtocol, CryptoProtocolFactory {
	static function factory() {
		if (extension_loaded('gnupg'))
			return new synd_crypto_gpgme();
		throw new RuntimeException("pecl/gnupg extension is not loaded");
	}
	
	function getProtocol() {
		return 'pgp';
	}

	function getDriver($protocol) {
		if ('pgp' == $protocol)
			return self::factory();
		return false;
	}

	function keyserver($uri) {
		return new synd_crypto_pgp_hpks($uri);
	}
}

/**
 * pecl/gnupg PGP driver
 * @link	http://pecl.php.net/package/gnupg
 */
class synd_crypto_gpgme extends synd_crypto_pgp {
	private $_gpg = null;
	private $_home = null;
	
	function __destruct() {
		if (null != $this->_home)
			SyndLib::unlink($this->_home, true);
	}
	
	function getHandle() {
		if (null == $this->_gpg) {
			$d = isset($_SERVER['SERVER_NAME']) ? $_SERVER['SERVER_NAME'] : 'local';
			$this->_gpg = new gnupg();
			$this->_home = "/tmp/$d/gnupg/".md5(uniqid(''));
			if (!SyndLib::createDirectory($this->_home))
				throw new RuntimeException("Chould not create temporary keyring directory '{$this->_home}'");
		}
		putenv("GNUPGHOME={$this->_home}");
		return $this->_gpg;
	}
	
	function key($buffer) {
		$gpg = $this->getHandle();
		$keyinfo = $gpg->import($buffer);
		if (!empty($keyinfo['fingerprint']))
			return new synd_crypto_gpgme_key($this, $keyinfo['fingerprint'], $buffer);
		return false;
	}
	
	function keyring($buffer) {
		require_once 'core/lib/crypto/Pem.class.inc';
		$gpg = $this->getHandle();
		$pems = Pem::parse($buffer);
		$result = array();
		
		foreach ($pems as $pem) {
			$buffer = $pem->toString();
			$keyinfo = $gpg->import($buffer);
			if (!empty($keyinfo['fingerprint']))
				$result[] = new synd_crypto_gpgme_key($this, $keyinfo['fingerprint'], $buffer);
		}
		
		return $result;
	}
	
	function sign(Keyserver $keyserver, MimePart $message) {}
	
	function verify(Keyserver $keyserver, MimePart $message, &$key, &$keyid) {
		$gpg = $this->getHandle();

		if ('multipart/signed' == strtolower($message->getHeader('Content-Type'))) {
			if (2 != count($parts = $message->getParts()) || 'application/pgp-signature' != strtolower($parts[1]->getHeader('Content-Type')))
				throw new InvalidArgumentException('Malformed PGP/MIME message');
			
			$text = $parts[0]->toString();
			$signature = $parts[1]->getContent();
			
			// Extract keyid/fingerprint from signature
			$result = $gpg->verify('', $signature, '');
			if (empty($result[0]['fingerprint']))
				throw new InvalidArgumentException('Could not extract keyid/fingerprint from signature');
			
			// Fetch key from keyserver
			$keyid = '0x'.substr($result[0]['fingerprint'], -8);
			if (false == ($key = $keyserver->fetch($this, $keyid)))
				return false;
			
			// Verify signature
			$result = $gpg->import($key->toString());
			if (empty($result['fingerprint']))
				throw new RuntimeException('Could not import key into temporary keyring');
			
			$result = $gpg->verify($text, $signature, '');
			if (!isset($result[0]['status']) || 0 !== $result[0]['status'])
				return false;

			return $parts[0];
		}
		else if ($message instanceof MimeTextpart_2_0_0) {
			$text = $message->getMessageBody();
			
			// Extract keyid/fingerprint from signature
			$result = $gpg->verify($text, false, '');
			if (empty($result[0]['fingerprint']))
				throw new InvalidArgumentException('Malformed signature, could not extract keyid');
			
			// Fetch key from keyserver
			$keyid = '0x'.substr($result[0]['fingerprint'], -8);
			if (false == ($key = $keyserver->fetch($this, $keyid)))
				return false;
			
			// Verify signature
			$result = $gpg->import($key->toString());
			if (empty($result['fingerprint']))
				throw new RuntimeException('Could not import key into temporary keyring');
			
			$cleartext = '';
			$result = $gpg->verify($text, false, $cleartext);
			if (!isset($result[0]['status']) || 0 !== $result[0]['status'])
				return false;

			$result = clone $message;
			$result->setContent($cleartext, false);
			return $result;
		}
		
		return false;
	}
	
	function encrypt(Keyserver $keyserver, MimePart $message) {}
	function decrypt(Keyserver $keyserver, MimePart $message) {}
}

/**
 * pecl/gnupg PGP key
 * @package		synd.core.lib
 */
class synd_crypto_gpgme_key implements CryptoKey {
	private $_driver = null;
	private $_fingerprint = null;
	private $_buffer = null;
	private $_keyinfo = null;
	
	function __construct(synd_crypto_gpgme $driver, $fingerprint, $buffer) {
		$this->_driver = $driver;
		$this->_fingerprint = $fingerprint;
		$this->_buffer = $buffer;
	}

	function toString() {
		return $this->_buffer;
	}
	
	private function getKeyinfo() {
		if (null === $this->_keyinfo) {
			$keyinfo = $this->_driver->getHandle()->keyinfo($this->_fingerprint);
			if (isset($keyinfo[0]))
				$this->_keyinfo = $keyinfo[0];
			else
				$this->_keyinfo = false;
		}
		return $this->_keyinfo;
	}

	function getKeyid() {
		$keyinfo = $this->getKeyinfo();

		if (!empty($keyinfo['subkeys'])) {
			foreach ($keyinfo['subkeys'] as $key) {
				if (isset($key['fingerprint']))
					return '0x'.substr($key['fingerprint'], -8);
				if (isset($key['keyid']))
					return $key['keyid'];
			}
		}

		return null;
	}

	function getFingerprint() {
		$keyinfo = $this->getKeyinfo();
		$result = array();
		
		if (!empty($keyinfo['subkeys'])) {
			foreach ($keyinfo['subkeys'] as $key) {
				if (isset($key['fingerprint']))
					return $key['fingerprint'];
			}
		}
		
		return false;
	}
	
	function getExpiry() {
		$keyinfo = $this->getKeyinfo();

		if (!empty($keyinfo['subkeys'])) {
			foreach ($keyinfo['subkeys'] as $key) {
				if (isset($key['expires']))
					return $key['expires'];
			}
		}

		return null;
	}
	
	function getCreated() {
		$keyinfo = $this->getKeyinfo();

		if (!empty($keyinfo['subkeys'])) {
			foreach ($keyinfo['subkeys'] as $key) {
				if (isset($key['timestamp']))
					return $key['timestamp'];
			}
		}

		return null;
	}

	function getIdentity() {
		$keyinfo = $this->getKeyinfo();
		if (empty($keyinfo['uids'])) 
			return new SimpleCryptoIdentity('Unknown', '');
		$info = reset($keyinfo['uids']);
		return new SimpleCryptoIdentity(Mime::charset($info['name'], 'utf-8'), $info['email']);
	}

	function getIdentities() {
		$keyinfo = $this->getKeyinfo();
		$result = array();
		
		if (!empty($keyinfo['uids'])) {
			foreach ($keyinfo['uids'] as $info) 
				$result[] = new SimpleCryptoIdentity(Mime::charset($info['name'], 'utf-8'), $info['email']);
		}
		
		return $result;
	}
	
	function getTrust() {
		return 0;
	}

	function setTrust($trust) {}

	function getProtocol() {
		return 'pgp';
	}

	function isRevoked() {
		$keyinfo = $this->getKeyinfo();
		return !empty($keyinfo['revoked']);
	}
	
	function isSigningKey() {
		$keyinfo = $this->getKeyinfo();

		if (!empty($keyinfo['subkeys'])) {
			foreach ($keyinfo['subkeys'] as $key) {
				if (isset($key['fingerprint']) && !empty($key['can_sign']) && !empty($key['is_secret']))
					return true;
			}
		}
		
		return false;
	}
	
	function isVerificationKey() {
		$keyinfo = $this->getKeyinfo();
		return !empty($keyinfo['can_sign']);
	}
	
	function isEncryptionKey() {
		$keyinfo = $this->getKeyinfo();
		return !empty($keyinfo['can_encrypt']);
	}
	
	function isDecryptionKey() {
		$keyinfo = $this->getKeyinfo();
		return !empty($keyinfo['is_secret']);
	}
}

/**
 * Interface to a HPKS keyserver
 * @package		synd.core.lib
 */
class synd_crypto_pgp_hpks implements Keyserver {
	protected $_uri = null;
	protected $_connection = null;
	
	function __construct($uri) {
		$this->_uri = $uri;
	}
	
	protected function getConnection() {
		if (null == $this->_connection) {
			require_once 'core/lib/HttpConnection.class.inc';
			$this->_connection = new HttpConnection($this->_uri);
		}
		return $this->_connection;
	}
	
	function fetch(CryptoProtocolFactory $factory, $keyid) {
		profiler_enter(__METHOD__);

		$response = $this->getConnection()->getRequest('/pks/lookup', array('op'=>'get','search'=>$keyid));
		if (null == $response) {
			profiler_leave(__METHOD__);
			return false;
		}
		
		$pems = Pem::parse($response);
		if (empty($pems)) {
			profiler_leave(__METHOD__);
			return false;
		}

		$key = $factory->getDriver('pgp')->key(reset($pems)->toString());
		profiler_leave(__METHOD__);
		
		return $key;
	}
	
	function store(CryptoKey $key) {}
	function delete(CryptoKey $key) {}

	function search(CryptoProtocolFactory $factory, $query = null, $offset = 0, $limit = 50) {
		$response = $this->getConnection()->getRequest('/pks/lookup', array('op'=>'get','search'=>$query));
		if (null == $response) 
			return false;
		
		$pems = Pem::parse($response);
		$keys = array();

		$driver = $factory->getDriver('pgp');
		foreach ($pems as $pem)
			$keys[] = $driver->key($pem->toString());
		
		return $keys;
	}
}
