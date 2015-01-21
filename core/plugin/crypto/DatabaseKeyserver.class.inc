<?php
require_once 'core/lib/crypto/Keyserver.class.inc';
require_once 'core/lib/crypto/CryptoProtocol.class.inc';

/**
 * Database backed keyserver implementation
 * @access		public
 * @package		synd.core.model
 */
class DatabaseKeyserver implements Keyserver {
	protected $_project = null;
	protected $_db = null;
	
	function __construct($project) {
		$this->_project = $project;
		$this->_db = $project->_storage->getDatabase();
	}
	
	protected function getBranch() {
		$result = array();
		for ($parent = $this->_project; !$parent->isNull(); $parent = $parent->getParent())
			$result[] = $parent->nodeId;
		return $result;
	}

	function fetch(CryptoProtocolFactory $factory, $keyid) {
		$sql = "
			SELECT k.PROTOCOL, k.INFO_TRUST, k.DATA_CONTENT FROM synd_crypto_key k
			WHERE
				k.project_node_id IN (".implode(',', $this->_db->quote($this->getBranch())).") AND
				k.keyid = ".$this->_db->quote($keyid);
		
		if (null != ($row = $this->_db->getRow($sql))) {
			$driver = $factory->getDriver($row['PROTOCOL']);
			if (false != ($key = $driver->key($row['DATA_CONTENT'])))
				return new DatabaseCryptoKey($key, $row['INFO_TRUST']);
		}
		
		return false;
	}
	
	function store(CryptoKey $key) {
		$this->delete($key);

		$identities = $key->getIdentities();
		$identity = reset($identities);

		$kid = $this->_db->nextId('synd_crypto_key');						
		$row = array(
			'KID'				=> $kid,
			'PROJECT_NODE_ID'	=> $this->_project->nodeId,
			'PROTOCOL'			=> $key->getProtocol(),
			'KEYID'				=> $key->getKeyid(),
			'FINGERPRINT'		=> $key->getFingerprint(),
			'FLAG_SIGN'			=> (int)$key->isSigningKey(),
			'FLAG_VERIFY'		=> (int)$key->isVerificationKey(),
			'FLAG_ENCRYPT'		=> (int)$key->isEncryptionKey(),
			'FLAG_DECRYPT'		=> (int)$key->isDecryptionKey(),
			'INFO_TRUST'		=> $key->getTrust(),
			'INFO_NAME'			=> $identity ? $identity->getName() : null,
			'INFO_EMAIL'		=> $identity ? $identity->getEmail() : null,
			'DATA_CONTENT'		=> $key->toString());
		$result = $this->_db->insert('synd_crypto_key', $row);
		
		foreach ($identities as $identity) {
			$row = array(
				'KID'			=> $kid,
				'INFO_NAME'		=> $identity->getName(),
				'INFO_EMAIL'	=> $identity->getEmail());
			$this->_db->insert('synd_crypto_identity', $row);
		}
	}

	function delete(CryptoKey $key) {
		$keyid = $key->getKeyid();
		$fingerprint = $key->getFingerprint();
		if (null == $keyid || null == $fingerprint) 
			throw new InvalidArgumentException('No fingerprint or keyids found in key');
		
		$sql = "
			DELETE FROM synd_crypto_key
			WHERE 
				project_node_id = ".$this->_db->quote($this->_project->nodeId)." AND
				(keyid = ".$this->_db->quote($keyid)." OR fingerprint = ".$this->_db->quote($key->getFingerprint()).")";
		$this->_db->query($sql);
	}

	function search(CryptoProtocolFactory $factory, $query = null, $offset = 0, $limit = 50) {
		if (null != $query) {
			$q = strtolower(trim($query));
			$sql = "
				FROM synd_crypto_key k
					LEFT JOIN (synd_crypto_identity id)
					ON (id.kid = k.kid)
				WHERE
					k.project_node_id IN (".implode(',', $this->_db->quote($this->getBranch())).") AND
					(LOWER(k.keyid) = ".$this->_db->quote($q)." OR
					 LOWER(id.info_name) LIKE ".$this->_db->quote("$q%")." OR
					 LOWER(id.info_email) LIKE ".$this->_db->quote("$q%").")";
			
			$sql1 = "SELECT COUNT(DISTINCT k.kid) $sql";
			$sql2 = "
				SELECT DISTINCT k.KID, k.INFO_TRUST, k.PROTOCOL, k.INFO_NAME, k.INFO_EMAIL, k.DATA_CONTENT 
				$sql
				ORDER BY k.info_name, k.info_email";
		}
		else {
			$sql1 = "
				SELECT COUNT(1) FROM synd_crypto_key k
				WHERE k.project_node_id IN (".implode(',', $this->_db->quote($this->getBranch())).")";
			$sql2 = "
				SELECT k.PROTOCOL, k.INFO_TRUST, k.INFO_NAME, k.INFO_EMAIL, k.DATA_CONTENT FROM synd_crypto_key k
				WHERE k.project_node_id IN (".implode(',', $this->_db->quote($this->getBranch())).")
				ORDER BY k.info_name, k.info_email";
		}
		
		$count = $this->_db->getOne($sql1);
		$keys = array();
		
		if ($count) {
			foreach ($this->_db->getAll($sql2, $offset, $limit) as $row) {
				if (false != ($driver = $factory->getDriver($row['PROTOCOL'])) && 
					false != ($key = $driver->key($row['DATA_CONTENT'])))
					$keys[] = new DatabaseCryptoKey($key, $row['INFO_TRUST']);
			}
		}
		
		return array($keys, $count);
	}
}

class DatabaseCryptoKey extends ForwardingCryptoKey {
	protected $_trust = null;
	
	function __construct(CryptoKey $next, $trust) {
		parent::__construct($next);
		$this->_trust = $trust;
	}
	
	function getTrust() {
		return $this->_trust;
	}
	
	function setTrust($trust) {
		$this->_trust = $trust;
	}
}
