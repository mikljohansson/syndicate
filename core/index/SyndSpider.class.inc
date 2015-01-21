<?php
require_once 'core/index/SyndEventProducer.class.inc';
require_once 'core/lib/AsyncHttpConnection.class.inc';

/**
 * Web spider/crawler
 *
 * Provide the spider with an index and a document queue and it will
 * crawl the sites and index their content.
 *
 */
class SyndSpider extends SyndEventProducer {
	/**
	 * Queue of uri's to be indexed
	 * @var		object	ISyndSpiderQueue
	 * @access	protected
	 */
	var $_queue = null;

	/**
	 * Index backend
	 * @var		object	ISyndWebIndex
	 * @access	protected
	 */
	var $_backend = null;

	/**
	 * @access	private
	 */
	var $_uri = null;

	/**
	 * @access	private
	 */
	var $_connection = null;
	
	/**
	 * @access	private
	 */
	var $_connections = null;
	
	/**
	 * Buffered document delete requests
	 * @access	private
	 */
	var $_delete = array();
	
	/**
	 * @access	private
	 */
	var $_maxConnections = 25;
	
	/**
	 * Min interval between requests to the same server in seconds (microseconds supported)
	 * @var		float
	 * @access	private
	 */
	var $_minInterval = 1;
	
	/**
	 * Spider contructor
	 *
	 * The seconds to wait between requests to the same server 
	 * parameter supports microseconds. However it is impolite to set
	 * it below 1 second and doing so might cause denial of service.
	 *
	 * @param	ISyndSpiderQueue
	 * @param	ISyndWebIndex
	 * @param	float	Seconds between requests to same server
	 */
	function SyndSpider(&$queue, $backend, $minInterval = 1) {
		$this->_queue = $queue;
		$this->_backend = $backend;
		$this->_minInterval = $minInterval;
		
		$this->attachHook('response', array($this, '_callback_response'));
		$this->_backend->attachHook('document_page', array($this, '_callback_document_page'), 10);
	}
	
	function getConnection($uri) {
		$info = parse_url($uri);
		if (isset($this->_connections[$info['host']])) {
			if (false === ($idle = $this->_connections[$info['host']]->getIdletime()) || $idle < $this->_minInterval)
				return false;
		}
		else {
			if (count($this->_connections) >= $this->_maxConnections)
				$this->_pruneConnections();
			if (count($this->_connections) >= $this->_maxConnections)
				return false;

			$this->_connections[$info['host']] = new AsyncHttpConnection($uri, $this);
			$this->_connections[$info['host']]->setRedirectCallback(array($this, '_callback_connection_redirect'));
		}
		
		return $this->_connections[$info['host']];
	}
	
	/**
	 * Find 10% idle connections to drop
	 * @access	private
	 */
	function _pruneConnections() {
		$idle = array_filter(SyndLib::invoke($this->_connections, 'getIdletime'));
		arsort($idle);

		foreach (array_slice(array_keys($idle),0,ceil($this->_maxConnections/5)) as $key)
			unset($this->_connections[$key]);
	}
	
	function getIdleConnections() {
		return SyndLib::filter($this->_connections, 'getIdletime');
	}
	
	/**
	 * Run the spider
	 */
	function run() {
		$uri = null;
		$count = 0;

		while (null != ($uri = $this->_queue->getLocation()) || 
			count($this->getIdleConnections()) < count($this->_connections)) {
			if (null != $uri) {
				if (false === ($connection = $this->getConnection($uri))) 
					$this->_queue->pushLocation($uri);
				else {
					print "GET $uri\n";

					$connection->getRequest($uri);
					print "OPEN ".count($this->_connections)." connections, ".count($this->getIdleConnections())." idle.\n";
					
					$uri = null;
					$count++;
				}
			}
			
			AsyncHttpConnection::select();
		}
		
		print "Spider finished, requested $count uris\n";
	}

	function httpResponse($connection, $response) {
		if (null == $response) {
			print "EMPTY ".$connection->getLocation()."\n";
			return;
		}
		
		print "RESP ".$connection->getLocation()."\n";

		// Delete documents which returned error codes (status != {200,302})
		if ('2' != substr($connection->getStatus(), 0, 1) &&
			'3' != substr($connection->getStatus(), 0, 1)) {
			if (null != ($uri = $connection->getLocation()))
				$this->_delete[] = $uri;
			if (count($this->_delete) > 25) {
				$this->_backend->delDocumentBatch($this->_delete);
				$this->_delete = array();
			}
			return;
		}

		// Run the response hook to index document
		$headers = $connection->getHeaders();
		$location = $connection->getLocation();

		if (isset($headers['content-type']))
			$type = strtolower(trim(reset(explode(';', $headers['content-type']))));
		if (empty($type)) {
			$parsed = parse_url($location);
			if (!isset($parsed['path']) || 
				null == ($type = strtolower(SyndLib::fileExtension($parsed['path'])))) {
				trigger_error('Could not determine a content type', E_USER_WARNING);
				return;
			}
		}
		
		$this->runHook('response', array($connection, $type, $location, $response));
	}

	function httpError($connection) {
		print "ERROR: ".$connection->getLocation()."\n";
	}
	
	function _callback_response(&$result, $connection, $type, $uri, $response) {
		// Extract document structure and text from response
		$document = $this->_backend->getDocumentStructure(
			$type, $uri, $response, $title, $content);
		if (null == $uri || null == $document)
			return null;
			
		// Extract modification date
		$headers = $connection->getHeaders();
		if (!isset($headers['last-modified']) || -1 == ($modified = strtotime($headers['last-modified'])))
			$modified = 0;
		$document->setModified($modified);
		
		$this->_uri = $uri;
		$this->_connection = $connection;

		// Add document to index
		$this->_backend->addDocument($uri, $title, $content, $document, strlen($response), $modified);

		// Check for 'Location:' header
		if (isset($headers['location'])) {
			if (false === ($i = strpos($headers['location'], ':')) || $i > 6)
				$uri = $connection->relative($uri, $headers['location']);
			if (!$this->runHook('location', array(&$uri)))
				$this->_queue->addLocation(rtrim(str_replace(' ', '%20', $uri), '?&'));
		}
	}

	function _callback_connection_redirect($uri) {
		return false;
	}
	
	function _callback_document_page(&$result, $document) {
		$links = $document->getLinks();
		$changed = false;

		// Run the location hook to add found uris to queue
		foreach ($links as $key => $uri) {
			if (false === ($i = strpos($uri, ':')) || $i > 6)
				$uri = $this->_connection->relative($this->_uri, $uri);

			if (!$this->runHook('location', array(&$uri)))
				$this->_queue->addLocation(rtrim(str_replace(' ', '%20', $uri), '?&'));
			else {
				unset($links[$key]);
				$changed = true;
			}
		}
		
		if ($changed)
			$document->setLinks($links);
	}
}

/**
 * Queue interface for the spider framework
 *
 * @abstract
 * @access		public
 * @package		synd.core.index
 */
class ISyndSpiderQueue {
	/**
	 * Adds an URI to be indexed
	 * @param	string	The URI to add
	 */
	function addLocation($uri) {}

	/**
	 * Adds multiple URIs to be indexed
	 * @param	array	The URIs to add
	 */
	function addLocations($uris) {}
	
	/**
	 * Returns the next URI to be indexed
	 * @return	string
	 */
	function getLocation() {}
	
	/**
	 * Returns a batch of URIs to be indexed
	 * @return	array
	 */
	function getLocations($limit) {}
}

/**
 * Used to buffer accesses to a spider queue
 * @see		SyndWebSpiderQueue
 */
class SyndBufferedSpiderQueue extends ISyndSpiderQueue {
	/**
	 * Backend queue to be cached
	 * @var		object	ISyndSpiderQueue
	 * @access	protected
	 */
	var $_backend = null;

	/**
	 * Buffered queue of URI's to be indexed
	 * @access	protected
	 */
	var $_queue = array();

	/**
	 * Discovered URI's; not yet checked if already indexed
	 * @access	protected
	 */
	var $_locations = array();
	
	/**
	 * @param	ISyndSpiderQueue	Backend queue to be cached
	 */
	function SyndBufferedSpiderQueue($backend) {
		$this->_backend = $backend;
		SyndLib::attachHook('shutdown', array($this, 'flush'));
	}
	
	function addLocation($uri) {
		$this->_locations[crc32($uri)] = $uri;
		if (count($this->_locations) > 2500)
			$this->flush();
	}
	
	function pushLocation($uri) {
		$this->_queue[] = $uri;
	}
	
	function addLocations($uris) {
		foreach ($uris as $uri)
			$this->_locations[crc32($uri)] = $uri;
		if (count($this->_locations) > 2500)
			$this->flush();
	}
	
	function getLocation() {
		if (empty($this->_queue))
			$this->_buffer();
		return array_shift($this->_queue);
	}
	
	function getLocations($limit) {
		if (empty($this->_queue))
			$this->_buffer();
		return array_splice($this->_queue, 0, $limit);
	}
	
	function flush() {
		if (!empty($this->_locations)) {
			$this->_backend->addLocations($this->_locations);
			unset($this->_locations);
			$this->_locations = array();
		}
	}

	function _buffer() {
		$this->flush();
		$this->_queue = $this->_backend->getLocations(750);
		shuffle($this->_queue);
	}
}

/**
 * Extension interface for the spider framework
 *
 * @abstract
 * @access		public
 * @package		synd.core.index
 */
class SyndSpiderExtension {
	/**
	 * Spider we're attached to
	 * @var		object	SyndSpider
	 * @access	protected
	 */
	var $_spider = null;
	
	/**
	 * Attaches this extension to a spider.
	 * @param	SyndSpider	Spider to attach to
	 */
	function initialize($spider) {
		$this->_spider = $spider;
	}
}

/**
 * URI and Content-Type filter for the spider
 */
class SyndSpiderURIFilter extends SyndSpiderExtension {
	var $_uriFilters = null;
	var $_autoValid = null;
	var $_autoInvalid = null;
	var $_typeFilters = null;
	var $_rewrite = null;

	var $_invalid = array();

	var $_maxResponseSize = 4000000;
	
	/**
	 * Url regexp filter
	 *
	 * The url must match one of the $uri patterns to pass. If it 
	 * doesn't match any of the $valid patterns the Content-Type
	 * if fetched and matched againt the $type patterns.
	 *
	 * The content type is checked again after fetching the entire
	 * page, so make sure it mathes all document types that should 
	 * be indexed.
	 *
	 * @param	array	Url must match one of these
	 * @param	array	Known valid url filters (no need to do HEAD to check Content-Type)
	 * @param	array	Known invalid url filters (no need to do HEAD to check Content-Type)
	 * @param	array	Content type filters
	 * @param	array	Normalization filters (strip PHPSESSID, ...)
	 */
	function SyndSpiderURIFilter($uri, $valid, $invalid, $type, $rewrite = array()) {
		$this->_uriFilters = $uri;
		$this->_autoValid = $valid;
		$this->_autoInvalid = $invalid;
		
		assert('!empty($type)');
		$this->_typeFilters = $type;
		
		$this->_rewrite = $rewrite;
	}
	
	function initialize($spider) {
		parent::initialize($spider);
		$spider->attachHook('response', array($this, '_callback_response'), 10);
		$spider->attachHook('location', array($this, '_callback_location'), 10);
	}

	/**
	 * Check Content-Type before indexing document
	 */
	function _callback_response(&$result, $connection, $type, $uri, $response) {
		foreach ((array)$this->_typeFilters as $pattern) {
			if (preg_match($pattern, $type))
				return null;
		}
		return true;
	}
	
	/**
	 * Check URI before adding to queue
	 *
	 * Will do a HEAD to get the Content-Type of documents not known
	 * to be valid. Known to be valids will be added to queue and have
	 * their Content-Type checked later when the whole document is 
	 * fetched for indexing.
	 */
	function _callback_location(&$result, $uri) {
		// Check valid patterns
		if (!empty($this->_uriFilters)) {
			for (;;) {
				foreach ($this->_uriFilters as $pattern) {
					if (preg_match($pattern, $uri))
						break 2;
				}
//				print "FILTER:URI $uri\n";
				return $result = true;
			}
		}

		// Check for known good uri
		foreach ((array)$this->_autoValid as $pattern) {
			if (preg_match($pattern, $uri)) {
				$uri = $this->_rewrite($uri);
				return null;
			}
		}

		// Check for known invalid uri
		foreach ((array)$this->_autoInvalid as $pattern) {
			if (preg_match($pattern, $uri)) {
//				print "FILTER:AUTOINVALID $uri\n";
				return $result = true;
			}
		}
		
		return true;

		// Fetch and check Content-Type header
		$parsed = parse_url($result);
		if (!isset($parsed['host']))
			return $result = true;
		$key = $parsed['host'];
		if (isset($parsed['path']))
			$key .= $parsed['path'];

		// Fetch headers from server
		if (!isset($this->_invalid[$key])) {
			print "HEAD $uri\n";
			
			$this->_invalid[$key] = true;
			if (count($this->_invalid) >= 5000)
				array_splice($this->_invalid, 0, 500);

			$connection = $this->_spider->getConnection($uri);
			$headers = $connection->headRequest($uri);

			if ('2' == substr($connection->getStatus(), 0, 1) && !empty($headers['content-type']) && 
				(empty($headers['content-length']) || $headers['content-length'] < $this->_maxResponseSize)) {
				foreach ((array)$this->_typeFilters as $pattern) {
					if (preg_match($pattern, $headers['content-type'])) {
						$this->_invalid[$key] = false;
						break;
					}
				}
			}

			print "FILTER:INVALIDTYPE {$headers['content-type']} $uri\n";
		}
				
		if (!$this->_invalid[$key])
			$uri = $this->_rewrite($uri);

		return $result = $this->_invalid[$key];
	}
	
	function _rewrite($uri) {
		foreach ($this->_rewrite as $pattern => $replacement)
			$uri = preg_replace($pattern, $replacement, $uri);
		return $uri;
	}
}
