<?php

/**
 * Search backend interface
 * @package		synd.core.module
 * @uses		synd.core.index
 */
interface SearchBackend {
	/**
	 * Runs an index query against this backend
	 *
	 * Returns the estimated number of matches and the matches 
	 * included in the span bounded by the offset and limit 
	 * parameters. The matches are indexed by the docid for use
	 * in the RSet. For example:
	 *
	 * <code>
	 *   array(
	 *     array(
	 *       array('DOCID' => 8956, 'PAGEID' => 'issue.54689', 'SECTION' => 'n.issue', 'WEIGHT' => 12.1235),
	 *       array('DOCID' => 6506, 'PAGEID' => 'issue.35043', 'SECTION' => 'n.issue', 'WEIGHT' => 5.3521),
	 *       array('DOCID' => 2696, 'PAGEID' => 'issue.9861', 'SECTION' => 'n.issue', 'WEIGHT' => 2.5986)
	 *     245
	 *   );
	 * </code>
	 *
	 * @param	string	Query string
	 * @param	array	Index sections to query
	 * @param	array	Security attributes to include
	 * @param	integer	Offset in result set to start from
	 * @param	integer	Max number of rows to return
	 * @param	string	Implicit operator to use
	 * @param	array	Docids of documents deemed relevant
	 * @return	array
	 */
	function getMatchSet($query, $sections, $attributes, $offset, $limit, $operator = '&&', $rset = null, $rsetdocids = true);
	
	/**
	 * Returns the estimated number of matches 
	 * @param	string	Query string
	 * @param	array	Index sections to query
	 * @param	array	Security attributes to include
	 * @param	string	Implicit operator to use
	 * @return	array
	 */
	function getMatchCount($query, $sections, $attributes, $operator = '&&');

	/**
	 * Fetches document meta data
	 * @param	array	List of docids
	 * @return	array
	 */
	function getDocuments($docids);

	/**
	 * Checks for alternate spellings
	 *
	 * <pre>
	 * Returns a list like 
	 *	array(
	 *		'speling' => 'spelling', 
	 *		'errror' => 'error')
	 * </pre>
	 *
	 * @param	array	Terms to check
	 * @return	array
	 */
	function getSpellingSuggestions($terms);

	/**
	 * Clear the document refresh queue
	 */
	function clearRefreshQueue();

	/**
	 * Is the indexer running
	 * @param	string	Clsid to check indexing, if null check status on document refresh indexer
	 * @return	bool
	 */
	function isIndexing($clsid = null);

	/**
	 * Forcibly start the document indexer
	 * @param	string		Namespace of queue
	 * @param	integer		Cluster key of this backend
	 * @param	integer		Union of all backend keys
	 * @param	array		URN of other backends to notify indexed by their cluster key
	 */
	function runIndexer($namespace, $key, $keys, $urns = null);
	
	/**
	 * Forcibly start the document indexer
	 * @param	string	Clsid to start indexer for, if null start the document refresh indexer
	 */
	function runClassIndexer($clsid);
	
	/**
	 * Fetch index statistics
	 *
	 * <code>
	 *  array(
	 *		'refresh_queue_size' => 123,
	 *		'sections' => array(
	 *			'n.issue' => 15634
	 *			'n.instance.item' => 13644,
	 *			'n.lease' => 1670
	 *			),
	 *		'last_analyze_begin' => 1137031226,
	 *		'last_analyze_end' => 1137031231,
	 *		'last_optimize_begin' => 1137031212,
	 *		'last_optimize_end' => 1137031226,
	 *		),
	 *  )
	 * </code>
	 *
	 * @param	string		Namespace of queue
	 * @param	integer		Cluster key of this backend
	 * @param	array		List of sections to fetch statistics for
	 * @return	array
	 */
	function getStatistics($namespace, $key, $sections = null);
}