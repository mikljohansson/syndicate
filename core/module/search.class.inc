<?php
require_once 'core/i18n.inc';
require_once 'core/Module.class.inc';
require_once 'core/index/SearchBackend.class.inc';
require_once 'core/lib/rpc/RpcTransport.class.inc';
require_once 'core/model/DomainObject.class.inc';

/**
 * Search engine and indexer module
 *
 * Provides indexing and advanced search services via the various 
 * index implementations (synd.core.index).
 *
 * @access		protected
 * @package		synd.core.module
 * @uses		synd.core.index
 */
class synd_module_search extends Module implements SearchBackend {
	var $_index = null;
	var $_webIndex = null;
	
	var $_title = null;
	var $_db = null;
	
	/**
	 * Exit handler attached
	 * @access	private
	 */
	var $_exitHandler = false;

	/**
	 * Index queue buffer
	 * @access	private
	 */
	var $_queue = null;
	
	function __construct($config = null) {
		if (!isset($config['namespace']))
			$config['namespace'] = isset($_SERVER['SERVER_NAME']) ? $_SERVER['SERVER_NAME'] : 'localhost';
		
		parent::__construct($config);
		
		SyndLib::attachHook('event', array($this, '_callback_event'));
		SyndLib::attachHook('search', array($this, '_callback_search'));
		SyndLib::attachHook('spelling', array($this, '_callback_spelling'));

		SyndLib::attachHook('menu_admin', array($this, '_callback_menu_admin'));
		SyndLib::attachHook('header', array($this, '_callback_header'));
	}
	
	/**
	 * @access	protected
	 */
	function _hook_manage_permissions(&$result) {
		$result[] = $this;	
	}
	
	function getDescription() {
		return SyndLib::translate('Provides indexing and search services via the various index implementations.');
	}
	
	/**
	 * Appends the 'invoke' permission
	 */
	function getDefinedPermissions() {
		$perms = parent::getDefinedPermissions();
		$perms['invoke'] = SyndLib::translate('Allow complete RPC method invocation. The search module needs this permission to be able to communicate with other search modules.');
		return $perms;
	}
	
	/**
	 * Displays the title and load the search.css stylesheet
	 */
	function request(Request $request) {
		SyndLib::attachHook('html_head_title', array($this, '_callback_html_head_title'), -100);
		tpl_load_stylesheet(tpl_design_uri('module/search/search.css'));
		return parent::request($request);
	}

	/**
	 * @access	protected
	 */
	function _callback_html_head_title(&$result) {
		return $result = null != $this->_title ? $this->_title : SyndLib::translate('Search');
	}

	function _callback_menu_admin(&$result, $menu) {
		if ($this->isPermitted('admin'))
			$menu->append(tpl_link('system','search','admin'), SyndLib::translate('Search'));
	}
	
	function _callback_header(&$result, Template $page) {
		if ($this->isPermitted('read'))
			$result .= $page->fetch(tpl_design_path('module/search/header_quicksearch_form.tpl'));
	}
	
	function _view_admin(Request $request, Template $page) {
		if (!$this->isPermitted('admin'))
			throw new ForbiddenException();
		$page->assign('request', $request);
		$page->assign('config', $this->_config);
		$page->assign('title', SyndLib::translate('Search engine control center'));
		return array('content' => $page->fetch('module/search/admin.tpl'));
	}

	function _view_help(Request $request, Template $page) {
		if (!$this->isPermitted('read'))
			throw new ForbiddenException();
		$page->assign('index', $this->getIndex());
		$page->assign('title', SyndLib::translate('Search help'));
		return array('content' => $page->fetch('module/search/help.tpl'));
	}

	function _callback_event(&$result, DomainObject $context, DomainEvent $event) {
		if (($event instanceof synd_event_create) || ($event instanceof synd_event_update) || ($event instanceof synd_event_delete)) {
			$subject = $event->getSubject();
			
			if (null != $subject->nodeId && $this->isIndexed($subject->classId())) {
				if (!$this->_exitHandler) {
					$this->_exitHandler = true;
					SyndLib::attachHook('shutdown', array($this, '_callback_flush'), -250);
				}

				$database = $this->getIndexDatabase();
				$this->_queue[] = array(
					'NAMESPACE'	=> $database->quote($this->_config['namespace']),
					'NODE_ID'	=> $database->quote($subject->nodeId));
			}
		}
	}
	
	/**
	 * Flushes the document refresh queue
	 * @access	private
	 */
	function _callback_flush(&$result) {
		if (!empty($this->_queue)) {
			$this->getIndexDatabase()->load('synd_search_queue', $this->_queue);
			$this->_queue = null;
			$this->_runIndexer();
		}
	}

	/**
	 * Starts the indexer on one backend
	 * @access	private
	 */
	function _runIndexer() {
		$urns = $this->getWritableBackends();
		$keys = array_keys($urns);
		shuffle($keys);
		
		if (empty($this->_config['backends'])) {
			$notify = array(); 
			$sum = (int)array_sum(array_keys($urns));
		}
		else {
			$notify = array();
			foreach ($this->_config['backends'] as $key => $urn)
				$notify[$key] = array($key, $urn);
			$sum = (int)array_sum(array_keys($this->_config['backends']));
		}
		
		foreach ($keys as $key) {
			unset($notify[$key]);
			if (null !== ($backend = Activator::getInstance($urns[$key])) && 
				false !== $backend->runIndexer($this->_config['namespace'], $key, $sum, $notify))
				break;
		}
	}
	
	function _callback_search(&$result, $clsid, $query, $order = 'date', $block = false, $operator = '&&') {
		global $synd_user;
		require_once 'core/lib/rpc/RpcTransport.class.inc';
		if ($order !== 'date' && $order !== 'relevance')
			$order = 'date';

		if ($this->isIndexed($clsid) && null != SyndNodeLib::loadClass($clsid)) {
			$sections = array('n.'.SyndNodeLib::getInheritedBranch($clsid));
			$attributes = SyndLib::runHook('index_security_attributes', array($synd_user));
			$result = SyndType::factory('indexed_collection', $this->getBackends(), 
				$query, $sections, $attributes, null, $operator, $order);
			return $block || $result->getCount();
		}
	}
	
	function _callback_spelling(&$result, $query) {
		if (preg_match_all("/\b[{$SYND_WORDCHARS}.]+(?!:)\b/", $query, $matches)) {
			$index = $this->getIndex();
			$result = $index->getSpellingSuggestions($matches[0]);
			return true;
		}
	}
	
	function _view_index(Request $request, Template $page) {
		global $synd_user;
		require_once 'core/lib/rpc/RpcTransport.class.inc';
		if (!$this->isPermitted('read'))
			throw new ForbiddenException();

		$query = isset($request['q']) ? trim($request['q']) : '';
		if ('' != $query)
			$page->assign('title', SyndLib::translate('Search: %s', tpl_chop(strip_tags($query), 25)));
		else
			$page->assign('title', SyndLib::translate('Search'));

		if (!empty($request['new'])) {
			if (isset($request['rset']))
				unset($request['rset']);
			if (isset($request['rsetpageid']))
				unset($request['rsetpageid']);
		}
		
		if ('' != $query || !empty($request['rset'])) {
			$sections = array('n','w');
			$attributes = SyndLib::runHook('index_security_attributes', array($synd_user));
			$offset = isset($_REQUEST['offset']) ? $_REQUEST['offset'] : 0;
			$limit = 10;
			$order = isset($_REQUEST['o']) ? $_REQUEST['o'] : 'date';
			
			if (null != ($uri = SyndLib::runHook('search_query', $query, $offset, $limit)))
				return array('redirect' => $uri);
			
			$rsetdocids = !empty($request['rsetdocids']);
			if (!isset($request['rset']))
				$rset = array();
			else if ($rsetdocids)
				$rset = array_keys((array)$request['rset']);
			else
				$rset = (array)$request['rset'];
			
			// Execute search and instantiate matches for nodes and webpages
			$mset = null;
			$ts1 = microtime(true);

			foreach ($this->getBackends() as $urn) {
				if (null !== ($backend = Activator::getInstance($urn)) && false !== ($mset = 
					$backend->getMatchSet($query, $sections, $attributes, $offset, $limit, '&&', $rset, $rsetdocids, $order)))
					break;
			}
			
			if (null !== $mset) {
				$result = array();
				$docids = array();
				$nids = array();

				// Split matches into document and nodes
				foreach ($mset[0] as $match) {
					if (0 === strpos($match['SECTION'], 'n.'))
						$nids[] = $match['PAGEID'];
					else
						$docids[] = $match['DOCID'];
				}

				// Batch load matches
				if (!empty($nids))
					$nodes = SyndLib::getInstances($nids);
				if (!empty($docids)) {
					foreach ($this->getBackends() as $urn) {
						if (null !== ($backend = Activator::getInstance($urn)) && 
							false !== ($documents = $backend->getDocuments($docids)))
							break;
					}
				}

				// Build result set
				foreach ($mset[0] as $match) {
					if (0 === strpos($match['SECTION'], 'n.')) {
						if (isset($nodes[$match['PAGEID']]) && $nodes[$match['PAGEID']]->isPermitted('read'))
							$result[$match['DOCID']] = $nodes[$match['PAGEID']];
					}
					else {
						if (isset($documents[$match['DOCID']]))
							$result[$match['DOCID']] = new synd_index_document($documents[$match['DOCID']]);
					}
				}

				$ts2 = microtime(true);

				// Build page
				$page->assign('offset', $offset);
				$page->assign('limit', $limit);
				$page->assign('count', $mset[1]);
				$page->assign('result', $result);
				$page->assign('duration', $ts2 - $ts1);

				// Extract keywords for highlighting and spelling suggestions
				preg_match_all("/\b[{$SYND_WORDCHARS}.]+(?!:)\b/", $query, $matches);
				$page->assign('highlight', $matches[0]);
				
				if (0 == $offset && $mset[1] < 10) {
					foreach ($this->getBackends() as $urn) {
						if (null !== ($backend = Activator::getInstance($urn)) && 	
							false !== ($suggestions = $backend->getSpellingSuggestions($matches[0]))) {
							$page->assign('suggestions', $suggestions);
							break;
						}
					}
				}
			}
		}
		
		$page->assign('request', $request);
		return array('content' => $page->fetch(tpl_design_path('module/search/index.tpl')));
	}

	/**
	 * Returns the database to store the indexes
	 * @return	Database
	 */
	function getIndexDatabase() {
		global $synd_maindb;
		if (null === $this->_db) {
			if (empty($this->_config['index']['queue'])) 
				$this->_db = $synd_maindb;
			else if (SyndLib::isError($this->_db = DatabaseManager::getConnection($this->_config['index']['queue']))) {
				trigger_error($this->_db->getMessage(), E_USER_WARNING);
				$this->_db = DatabaseManager::getConnection('null://null');
			}
		}
		return $this->_db;
	}
	
	/**
	 * Returns the node index
	 * @return	AbstractIndex
	 */
	function getIndex() {
		if (null === $this->_index) {
			if (isset($this->_config['index'])) 
				$this->_index = $this->createIndex($this->_config['index']);
			else {
				require_once 'core/index/driver/null.class.inc';
				$this->_index = new synd_index_null();
			}
		}
		return $this->_index;
	}
	
	private function createIndex($config) {
		global $synd_maindb;
		$clsid = $config['strategy'];
		$class = "synd_index_$clsid";

		require_once "core/index/driver/$clsid.class.inc";
		$index = new $class(isset($config['dsn']) ? $config['dsn'] : $synd_maindb);
		
		if (!empty($config['extensions'])) {
			foreach ($config['extensions'] as $info) {
				if (is_array($info)) {
					$clsid = array_shift($info);
					$class = "synd_plugin_$clsid";

					require_once "core/index/plugin/$clsid.class.inc";
					$extension = new $class(
						array_shift($info),
						array_shift($info),
						array_shift($info),
						array_shift($info),
						array_shift($info));
				}
				else {
					$class = "synd_plugin_$info";
					require_once "core/index/plugin/$info.class.inc";
					$extension = new $class();
				}
				
				if (($extension instanceof synd_plugin_field))
					Module::runHook('field_extension', array($extension));
				$index->loadExtension($extension);
			}
		}
		
		return $index;
	}

	function getClasses() {
		return (array)SyndLib::runHook('index_active_clsids');
	}
	
	protected function isIndexed($clsid) {
		return count(array_intersect($this->getClasses(), explode('.',SyndNodeLib::getInheritedBranch($clsid))));
	}
	
	protected function getScheduler() {
		require_once 'core/lib/SyndTaskHandler.class.inc';
		return SyndTaskHandler::factory('index');
	}
	
	protected function getBackends() {
		if (empty($this->_config['backends']))
			return array(0x01 => 'local;search');
		if (isset($this->_config['primary'], $this->_config['backends'][$this->_config['primary']]))
			return array($this->_config['primary'] => $this->_config['backends'][$this->_config['primary']]);
		$urns = $this->_config['backends'];
		shuffle($urns);
		return $urns;
	}

	protected function getWritableBackends() {
		if (empty($this->_config['backends']))
			return array(0x01 => 'local;search');
		if (isset($this->_config['primary'], $this->_config['backends'][$this->_config['primary']]))
			return array($this->_config['primary'] => $this->_config['backends'][$this->_config['primary']]);
		return $this->_config['backends'];
	}

	/**
	 * @see		SearchBackend::getMatchSet()
	 */
	function getMatchSet($query, $sections, $attributes, $offset, $limit, $operator = '&&', $rset = null, $rsetdocids = true, $order = 'date') {
		require_once 'core/index/IndexQuery.class.inc';
		$index = $this->getIndex();
		
		if (!empty($rset) && !$rsetdocids)
			$rset = $index->documentId($rset);

		$q = new IndexQuery($query, $sections, $attributes, empty($rset) ? null : $rset, $operator);
		if ('' == trim($query) && count($eset = $index->getExpandSet($q)))
			$q = new IndexQuery(implode(' ', $eset), $sections, $attributes, empty($rset) ? null : $rset, $operator);
		
		$matches = $index->getMatchSet($q, $offset, $limit, $order);
		$count = $index->getMatchCount($q);
		return array($matches, $count);
	}

	/**
	 * @see		SearchBackend::getMatchCount()
	 */
	function getMatchCount($query, $sections, $attributes, $operator = '&&') {
		require_once 'core/index/IndexQuery.class.inc';
		$query = new IndexQuery($query, $sections, $attributes, null, $operator);
		$index = $this->getIndex();
		return $index->getMatchCount($query);
	}

	/**
	 * @see		SearchBackend::getDocuments()
	 */
	function getDocuments($docids) {
		$index = $this->getIndex();
		return $index->getDocuments($docids);
	}

	/**
	 * @see		SearchBackend::getSpellingSuggestions()
	 */
	function getSpellingSuggestions($terms) {
		$index = $this->getIndex();
		return $index->getSpellingSuggestions($terms);
	}

	/**
	 * @see		SearchBackend::isIndexing()
	 */
	function isIndexing($clsid = null) {
		$handler = $this->getScheduler();
		return $handler->isScheduled(null == $clsid ? 'node_indexer_task' : "class_indexer_task.$clsid");
	}

	/**
	 * @see		SearchBackend::runIndexer()
	 */
	function runIndexer($namespace, $key, $keys, $urns = null) {
		$handler = $this->getScheduler();
		if (!$handler->isScheduled('node_indexer_task')) {
			$handler->appendTask(SyndType::factory(
				"node_indexer_task", $this->getIndex(), $namespace, $key, $keys, $urns), 
				"node_indexer_task");
		}
	}

	/**
	 * @see		SearchBackend::runClassIndexer()
	 */
	function runClassIndexer($clsid) {
		if (!SyndNodeLib::loadClass($clsid))
			return false;

		$order = isset($this->_config['order'][$clsid]) ? $this->_config['order'][$clsid] : array();
		$handler = $this->getScheduler();
		$handler->appendTask(SyndType::factory(
			"class_indexer_task", $this->getIndex(), $clsid, $order), 
			"class_indexer_task.$clsid");
	}

	/**
	 * @see		SearchBackend::clearRefreshQueue()
	 */
	function clearRefreshQueue() {
		$database = $this->getIndexDatabase();
		$database->query('DELETE FROM synd_search_queue');
	}
	
	/**
	 * @see		SearchBackend::getStatistics()
	 */
	function getStatistics($namespace, $key, $sections = null) {
		$index = $this->getIndex();
		$database = $this->getIndexDatabase();
		
		$sql = "
			SELECT COUNT(DISTINCT q.node_id) FROM synd_search_queue q
			WHERE 
				q.namespace = ".$database->quote($namespace)." AND
				".$database->bitand('q.backends',$database->quote($key))." = 0";

		$stats = array(
			'refresh_queue_size' => $database->getOne($sql),
			);
		
		foreach ((array)$sections as $section)
			$stats['sections'][$section] = $index->getDocumentCount($section);

		$stats['last_analyze_begin'] = $this->getVariable('lastAnalyzed');
		$stats['last_analyze_end'] = $this->getVariable('lastAnalyzedFinished');
		$stats['last_optimize_begin'] = $this->getVariable('lastOptimized');
		$stats['last_optimize_end'] = $this->getVariable('lastOptimizedFinished');

		return $stats;
	}
	
	/**
	 * @access	private
	 */
	function getVariable($name) {
		return SyndLib::runHook('variable_get', 'search', $name);
	}
	
	/**
	 * @access	private
	 */
	function setVariable($name, $value) {
		SyndLib::runHook('variable_set', 'search', $name, $value);
	}
	
	/**
	 * Runs the index optimization every week and index analyze every 24 hours
	 */
	function _hook_cron(&$result) {
		$lastOptimized = $this->getVariable('lastOptimized');
		if (null == $lastOptimized || $lastOptimized < time() - (60*60*24*5 - 60*60)) {
			$this->setVariable('lastOptimized', time());
			$index = $this->getIndex();
			$index->optimize();
			$this->setVariable('lastOptimizedFinished', time());
		}

		$lastAnalyzed = $this->getVariable('lastAnalyzed');
		if (null == $lastAnalyzed || $lastAnalyzed < time() - (60*60*23)) {
			$this->setVariable('lastAnalyzed', time());
			$index = $this->getIndex();
			$index->analyze();
			$this->setVariable('lastAnalyzedFinished', time());
		}
	}
	
	function _hook_search_help_uri(&$result) {
		return $result = tpl_link('search','help');
	}

	function _hook_field_extension(&$result, $extension) {
		if (!empty($this->_config['field_extension_hashes'])) {
			foreach ($this->_config['field_extension_hashes'] as $field => $alias) {
				if (null != $alias)
					$extension[0]->addFieldAlias($alias, $field);
				else
					$extension[0]->addField($field);
			}
		}
	}

	function _hook_field_extension_description(&$result, $field) {
		switch ($field) {
			case 'id':
				$result = SyndLib::translate('An unambiguous reference to the resource within a given context.');
				break;
			case 'body':
				$result = SyndLib::translate('Full document or resource text.');
				break;
			case 'title':
				$result = SyndLib::translate('Title given to the document or resource.');
				break;
			case 'creator':
				$result = SyndLib::translate('Person or entity primarily responsible for making the content of the resource.');
				break;
			case 'description':
				$result = SyndLib::translate('Description of the content of the resource.');
				break;
			case 'subject':
				$result = SyndLib::translate('A topic or keywords of the content of the resource.');
				break;
			case 'created':
				$result = SyndLib::translate('Date of creation of the resource.');
				break;
			case 'filetype':
				$result = SyndLib::translate("File format such as 'pdf' or 'doc' (ex. filetype:pdf)");
				break;
		}
	}
	
	function getSpiderPreloadLocations() {
		return (array)$this->getVariable('spider_preload_locations');
	}

	function getSpiderURIFilters() {
		return (array)$this->getVariable('spider_uri_filters');
	}

	function getContentTypeFilters() {
		$filters = array();
		$filters[] = '/^text/(html|plain)\b/iS';

		$types = array();
		$flags = $this->getSpiderFlags();
		
		if (!empty($flags['content']['pdf']))
			$types[] = 'pdf';
		if (!empty($flags['content']['doc']))
			$types[] = 'msword';

		if (!empty($flags['content']['xls'])) {
			$types[] = 'msexcel';
			$types[] = 'vnd\.ms-excel';
		}
		if (!empty($flags['content']['xls'])) {
			$types[] = 'msexcel';
			$types[] = 'vnd\.ms-excel';
		}
		if (!empty($flags['content']['ppt']))
			$types[] = 'vnd\.ms-powerpoint';
		if (!empty($flags['content']['ppt'])) {
			$types[] = 'vnd\.sun\.xml\.writer';
			$types[] = 'x-soffice';
		}
		
		while (!empty($types))
			$filters[] = '/^application\/('.implode('|',array_splice($types,0,5)).')\b/iS';
			
		return array_merge($filters, (array)$this->getVariable('spider_content_filters'));
	}

	function getSpiderFlags() {
		return (array)$this->getVariable('spider_flags');
	}

	function _view_display(Request $request, Template $page) {
		if (!$this->isPermitted('read'))
			throw new ForbiddenException();
		if (empty($request['uri']))
			throw new InvalidArgumentException();
		
		$index = $this->getWebIndex();
		if (null == ($document = $index->getDocument($request['uri'])))
			throw new NotFoundException();
		
		$page->assign('title', $document['TITLE']);
		$page->assign('request', $request);
		$page->assign('document', $document);
		return array('content' => $page->fetch(tpl_design_path('module/search/display.tpl')), 'page' => 'empty.tpl');
	}

	function _view_runIndexer(Request $request, Template $page) {
		if (!$this->isPermitted('admin'))
			throw new ForbiddenException();
		if (false === ($key = array_search($request['urn'], $this->_config['backends'])) ||
			null === ($backend = Activator::getInstance($request['urn'])))
			throw new InvalidArgumentException();
		$backend->runIndexer($this->_config['namespace'], (int)$key, (int)array_sum(array_keys($this->_config['backends'])));
	}

	function _view_runClassIndexer(Request $request, Template $page) {
		if (!$this->isPermitted('admin'))
			throw new ForbiddenException();
		if (false === ($key = array_search($request['urn'], $this->_config['backends'])) ||
			null === ($backend = Activator::getInstance($request['urn'])))
			throw new InvalidArgumentException();
		$backend->runClassIndexer($request[0]);
	}

	function _view_clearRefreshQueue(Request $request, Template $page) {
		if (!$this->isPermitted('admin'))
			throw new ForbiddenException();
		if (false === ($key = array_search($request['urn'], $this->_config['backends'])) ||
			null === ($backend = Activator::getInstance($request['urn'])))
			throw new InvalidArgumentException();
		$backend->clearRefreshQueue();
	}

	function _view_setSpiderOptions(Request $request, Template $page) {
		if (!$this->isPermitted('admin'))
			throw new ForbiddenException();
		
		if (!empty($request['filter']['domain'])) {
			$filter = '/^https?:\/\/';
			if (!empty($request['filter']['subdomains']))
				$filter .= '([\w\.\-]+?\.)?';
			$filter .= preg_quote($request['filter']['domain'], '/');
			$filter .= '(/|$)/iS';

			$filter = str_replace('*', '.*?', $filter);
			
			$filters = $this->getSpiderURIFilters();
			$filters[] = $filter;
			$this->setVariable('spider_uri_filters', $filters);
		}
		
		if (!empty($request['preload'])) {
			$locations = $this->getSpiderPreloadLocations();
			$locations[] = $request['preload'];
			$this->setVariable('spider_preload_locations', $locations);
		}

		$this->setVariable('spider_flags', $request['flags']);
	}

	function _view_delSpiderURIFilter(Request $request, Template $page) {
		if (!$this->isPermitted('admin'))
			throw new ForbiddenException();
		if (!isset($request['key']))
			return;

		$filters = $this->getSpiderURIFilters();
		unset($filters[$request['key']]);
		$this->setVariable('spider_uri_filters', $filters);
	}

	function _view_delSpiderPreloadLocation(Request $request, Template $page) {
		if (!$this->isPermitted('admin'))
			throw new ForbiddenException();
		if (!isset($request['key']))
			return;

		$locations = $this->getSpiderPreloadLocations();
		unset($locations[$request['key']]);
		$this->setVariable('spider_preload_locations', $locations);
	}
}

/**
 * @access	private
 */
class synd_index_document {
	var $_document = null;	
	
	function synd_index_document($document) {
		$this->_document = $document;
	}
	
	function id() {
		return $this->_document['URI'];
	}

	function getTitle() {
		return $this->_document['TITLE'];
	}
	
	function getContents() {
		return $this->_document['CONTENT'];
	}
	
	function getLocation() {
		return $this->_document['URI'];
	}
	
	function getModificationTime() {
		return $this->_document['MODIFIED'];
	}
	
	function getSize() {
		return $this->_document['SIZE'];
	}
}
