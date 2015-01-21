<?php
require_once 'core/lib/crypto/CryptoProtocol.class.inc';

/**
 * Cryptography keyserver interface
 * @access		public
 * @package		synd.core.lib
 */
interface Keyserver {
	/**
	 * Key must support signing
	 */
	const SIGN		= 0x01;

	/**
	 * Key must support verifying signatures
	 */
	const VERIFY	= 0x02;

	/**
	 * Key must support encryption
	 */
	const ENCRYPT	= 0x04;

	/**
	 * Key must support decryption
	 */
	const DECRYPT	= 0x08;
	
	/**
	 * Fetches a key from the server
	 * @param	CryptoProtocolFactory	Factory to instantiate protocol drivers
	 * @param	string					Key id to fetch
	 * @return	CryptoKey				Returns false on failure to find key
	 */
	function fetch(CryptoProtocolFactory $factory, $keyid);

	/**
	 * Stores a key
	 * @throws	RuntimeException
	 * @param	CryptoKey
	 */
	function store(CryptoKey $key);
	
	/**
	 * Delete a key
	 * @throws	RuntimeException
	 * @param	CryptoKey
	 */
	function delete(CryptoKey $key);

	/**
	 * Searches for keys
	 * @param	CryptoProtocolFactory	Factory to instantiate protocol drivers
	 * @param	string					Query to match, set to null to return all keys
	 * @param	integer					Result offset
	 * @param	integer					Max number of results
	 * @return	array					Matches at index 0, total numbers of matches at index 1
	 */
	function search(CryptoProtocolFactory $factory, $query = null, $offset = 0, $limit = 50);
}