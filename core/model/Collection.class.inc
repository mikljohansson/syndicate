<?php

/**
 * Collection interface
 *
 * Reports and other collection aware systems such as the SyncML 
 * module uses the collection interface to get hold of content.
 *
 * @abstract
 * @access		public
 * @package		synd.core.model
 */
interface Collection {
	/**
	 * Returns the name of this collection
	 */
	function toString();
	
	/**
	 * Returns number of items in this collection
	 * @return	integer
	 */
	function getCount();
	
	/**
	 * Returns the contents of this collection indexed by instance id
	 * @param	integer	Offset in result set to start from
	 * @param	integer	Max number of rows to return
	 * @return	array
	 */
	function getContents($offset = 0, $limit = null);
	
	/**
	 * Returns the number of items belonging to a set of classes
	 * @param	array	The classes to filter on
	 * @return	integer
	 */
	function getFilteredCount($classes);
	
	/**
	 * Returns the items belonging to a set of classes indexed by instance id
	 * @param	array	The classes to filter on
	 * @return	array
	 */
	function getFilteredContents($classes);
}
