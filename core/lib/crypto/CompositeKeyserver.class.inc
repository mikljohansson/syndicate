<?php
require_once 'core/lib/crypto/Keyserver.class.inc';

/**
 * Aggregates a primary and several secondary keyservers
 * @access		public
 * @package		synd.core.lib
 */
class CompositeKeyserver implements Keyserver {
	protected $_primary = null;
	protected $_secondaries = null;
	
	function __construct(Keyserver $primary, Array $secondaries) {
		$this->_primary = $primary;
		$this->_secondaries = $secondaries;
	}

	function fetch(CryptoProtocolFactory $factory, $keyid) {
		if (false != ($key = $this->_primary->fetch($factory, $keyid)))
			return $key;
		foreach ($this->_secondaries as $secondary) {
			if (false != ($key = $secondary->fetch($factory, $keyid))) {
				$this->_primary->store($key);
				return $key;
			}
		}
		return false;
	}

	function store(CryptoKey $key) {
		return $this->_primary->store($key);
	}
	
	function delete(CryptoKey $key) {
		return $this->_primary->delete($key);
	}

	function search(CryptoProtocolFactory $factory, $query = null, $offset = 0, $limit = 50) {
		return $this->_primary->search($factory, $query, $offset, $limit);
	}
}
