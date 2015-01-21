<?php

/**
 * Large object storage device
 *
 * The storage device allows clients to store and retrive files and 
 * other lobs based on namespace and key.
 *
 * @abstract
 * @access		public
 * @package		synd.core.model
 */
class LobStorageDevice {
	/**
	 * Stores a LOB on this storage syncronously
	 *
	 * Device should ensure that the provided checksum matches the 
	 * data written and return false if inconsistencies are found
	 *
	 * @param	string	Namespace
	 * @param	string	Namespace unique key
	 * @param	string	URI of LOB to store
	 * @param	string	MD5 checksum of LOB contents
	 * @param	integer	Size of LOB in bytes
	 * @return	bool	Returns true on success, false on failure
	 */
	function set($namespace, $variable, $uri, $checksum, $size) {}

	/**
	 * Stores a LOB buffer on this storage syncronously
	 *
	 * Device should ensure that the provided checksum matches the 
	 * data written and return false if inconsistencies are found
	 *
	 * @param	string	Namespace
	 * @param	string	Namespace unique key
	 * @param	string	LOB contents to store
	 * @param	string	MD5 checksum of LOB contents
	 * @return	bool	Returns true on success, false on failure
	 */
	function put($namespace, $variable, $buffer, $checksum) {}

	/**
	 * Stores a LOB on this storage asyncronously
	 *
	 * The callback will be run like
	 *  callback(LobStorageDevice device, string namespace, string variable, string checksum, mixed userarg)
	 *
	 * @param	string	Namespace
	 * @param	string	Namespace unique key
	 * @param	string	URI of LOB to store
	 * @param	string	MD5 checksum of LOB contents
	 * @param	integer	Size of LOB in bytes
	 * @param	array	Callback to run on completion as array(object listener, string method)
	 * @param	mixed	Argument to pass to callback
	 * @return	bool	Returns true on success, false on failure
	 */
	function replicate($namespace, $variable, $uri, $checksum, $size, $callback, $userarg = null) {}

	/**
	 * Returns the URI of a LOB in this storage
	 * @param	string	Namespace
	 * @param	string	Namespace unique key
	 * @param	integer	Expected LOB size
	 * @return	string	Returns null on not found, false on failure
	 */
	function get($namespace, $variable, $size = null) {}
	
	/**
	 * Deletes a LOB from this storage
	 * @param	string	Namespace
	 * @param	string	Namespace unique key
	 * @return	bool	Returns true on success, false on failure
	 */
	function delete($namespace, $variable) {}
	
	/**
	 * Checksums a list of LOBs
	 * @param	string	Namespace
	 * @param	string	List of LOB ids
	 * @return	array	Returns an array of checksums indexed by LOB id
	 */
	function checksum($namespace, $variables) {}
	
	/**
	 * Returns total device space in bytes
	 * @return	int64
	 */
	function getTotalSpace() {}

	/**
	 * Returns unallocated device space in bytes
	 * @return	int64
	 */
	function getFreeSpace() {}
}
