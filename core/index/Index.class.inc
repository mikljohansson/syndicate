<?php
require_once 'core/index/IndexQuery.class.inc';

/**
 * Search engine index interface
 * @access		public
 * @package		synd.core.index
 */
interface Index {
	/**
	 * Returns an index builder that can be used to modify the index
	 * @return	IndexBuilder
	 */
	function getIndexer();

	/** 
	 * Executes a search query.
	 *
	 * Returns the weighted document ids in a list as 
	 *
	 * <code>
	 *	array(
	 *		'DOCID' 	=> <int>, 
	 *		'PAGEID'	=> <string),
	 *		'SECTION'	=> <string>, 
	 *		'WEIGHT'	=> <double>)
	 * </code>
	 *
	 * @param	IndexQuery	Query object
	 * @param	integer		Rows index to start fetching on
	 * @param	integer		Max number of rows to return
	 * @return	array
	 */
	function getMatchSet(IndexQuery $query, $offset, $limit);

	/** 
	 * Returns the number of matching documents
	 * @param	IndexQuery	Query object
	 * @return	integer
	 */
	function getMatchCount(IndexQuery $query);

	/**
	 * Suggested a list of terms to expand the query
	 * @param	IndexQuery	Query object
	 * @param	integer		Max number of terms to return
	 * @return	array
	 */
	function getExpandSet(IndexQuery $query, $limit = 10);
}

/**
 * Objects that may be indexed implements this interface
 * @access		public
 * @package		synd.core.index
 */
interface Indexable {
	/**
	 * Allows the object to index itself by calling methods on the IndexBuilder interface
	 * @param	IndexBuilder	The active builder
	 */
	function acceptIndexer(IndexBuilder $indexer);
}
