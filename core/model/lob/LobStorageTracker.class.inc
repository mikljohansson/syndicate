<?php

/**
 * Large object storage tracker
 *
 * The storage tracker allows clients to store and retrive files and
 * other lobs based on namespace and key. It coordinates a cluster of
 * storage devices, propagates replicas and heals failed devices.
 *
 * @abstract
 * @access		public
 * @package		synd.core.model
 */
class LobStorageTracker {
	/**
	 * Stores a LOB on this storage pool
	 * @param	string	Namespace
	 * @param	string	Namespace unique key
	 * @param	string	URI of LOB to store
	 * @param	string	MD5 checksum of LOB contents
	 * @param	integer	Size of LOB in bytes
	 * @param	integer	Number of replicas to maintain
	 * @return	bool	Returns true on success, false on failure
	 */
	function set($namespace, $variable, $uri, $checksum, $size, $replicas) {}

	/**
	 * Stores a LOB buffer on this storage pool
	 * @param	string	Namespace
	 * @param	string	Namespace unique key
	 * @param	string	LOB contents to store
	 * @param	string	MD5 checksum of LOB contents
	 * @param	integer	Size of LOB in bytes
	 * @param	integer	Number of replicas to maintain
	 * @return	bool	Returns true on success, false on failure
	 */
	function put($namespace, $variable, $buffer, $checksum, $size, $replicas) {}

	/**
	 * Copies a LOB in this storage pool
	 * @param	string	Source namespace
	 * @param	string	Source LOB id
	 * @param	string	Target namespace
	 * @param	string	Target LOB id
	 * @param	integer	Number of replicas to maintain
	 * @return	bool	Returns true on success, false on failure
	 */
	function copy($sourcens, $sourcevar, $targetns, $targetvar) {}

	/**
	 * Returns the URI of a LOB in this storage pool
	 * @param	string	Namespace
	 * @param	string	Namespace unique key
	 * @return	string	Returns null on not found, false on failure
	 */
	function get($namespace, $variable) {}
	
	/**
	 * Deletes a LOB from this storage pool
	 * @param	string	Namespace
	 * @param	string	Namespace unique key
	 * @param	bool	Removes all variables beginning with $variable
	 * @return	bool	Returns true on success, false on failure
	 */
	function delete($namespace, $variable, $recursive = false) {}
	
	/**
	 * Returns inode information for a LOB
	 *
	 * <code>
	 *  array(
	 *    'dev' => 10,
	 *    'ino' => 2306,
	 *    'mode' => 33188,
	 *    'nlink' => 1,
	 *    'uid' => 72,
	 *    'gid' => 72,
	 *    'rdev' => 0,
	 *    'size' => 92847,
	 *    'atime' => 1139223861,
	 *    'mtime' => 1139223861,
	 *    'ctime' => 1139223561,
	 *    'blksize' => -1,
	 *    'blocks' => 1,
	 *  )
	 * </code>
	 *
	 * @link	http://www.php.net/stat
	 * @param	string	Namespace
	 * @param	string	Namespace unique key
	 * @return	array
	 */
	function stat($namespace, $variable) {}
}
