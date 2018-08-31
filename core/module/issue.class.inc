<?php
require_once 'core/module/node.class.inc';
require_once 'core/model/node/project.class.inc';
require_once 'core/model/node/issue.class.inc';

/**
 * Issue tracking and project managment module.
 * @access		protected
 * @package		synd.core.module
 */
class synd_module_issue extends synd_module_node {
	/**
	 * @access	private
	 */
	var $_title = null;

	function __construct($config = null) {
		parent::__construct($config);
		SyndLib::attachHook('index_active_clsids', array($this, '_callback_index_active_clsids'));
		SyndLib::attachHook('index_security_attributes', array($this, '_callback_index_security_attributes'));
		SyndLib::attachHook('user_summary_default', array($this, '_callback_user_summary_default'));
		SyndLib::attachHook('user_summary_result', array($this, '_callback_user_summary_result'));
		SyndLib::attachHook('search_query', array($this, '_callback_search_query'));
		SyndLib::attachHook('search_result', array($this, '_callback_search_result'));
		SyndLib::attachHook('menu', array($this, '_callback_menu'));
		SyndLib::attachHook('menu_admin', array($this, '_callback_menu_admin'));
		SyndLib::attachHook('syncronize', array($this, '_callback_syncronize'));
		SyndLib::attachHook('default_request_handler', array($this, '_callback_default_request_handler'));
	}
	
	/**
	 * Loads the issue.css stylesheet and displays the sidebar
	 */
	function request(Request $request) {
		if ($this->isPermitted('read')) {
			SyndLib::attachHook('breadcrumbs', array($this, '_callback_breadcrumbs'));
			SyndLib::attachHook('block', array($this, '_callback_block'));
		}

		tpl_load_stylesheet(tpl_design_uri('module/issue/issue.css'));
		tpl_load_script(tpl_design_uri('js/ole.js'));

		// Translate /issue/123/ style uris
		if (isset($request[0]) && is_numeric($request[0]) && 
			null != ($issue = synd_node_issue::findIssue(SyndNodeLib::getDefaultStorage('issue'), $request[0]))) {
			$request->setContext(array_merge(array('view',$issue->nodeId), array_slice($request->getContext(), 1)));
			return parent::request($request);
		}

		return parent::request($request);			
	}
	
	function _callback_html_head_title(&$result) {
		if (null == $this->_title)
			return parent::_callback_html_head_title($result);
		return $result = $this->_title;
	}
	
	function _callback_menu(&$result, $menu) {
		if ($this->isPermitted('read'))
			$menu->append(tpl_link('issue'), SyndLib::translate('Issues'), '', array($this, '_callback_menu_default'), array('','issue','issue/*'));
	}
	
	function _callback_menu_admin(&$result, $menu) {
		if ($this->isPermitted('admin'))
			$menu->append(tpl_link('system','issue','admin'), SyndLib::translate('Issues'));
	}
	
	function _callback_menu_default($menu) {
		$menu->append(tpl_link('issue','issues'), SyndLib::translate('My issues'), 
			SyndLib::translate('Accesskey: %s','Q'), null, null, 'q');
		$menu->append(tpl_link('issue','unassigned'), SyndLib::translate('Unassigned'), 
			SyndLib::translate('Accesskey: %s','W'), null, null, 'w');
		$menu->append(tpl_link('issue','report'), SyndLib::translate('Report'));
		$menu->append(tpl_link('issue','search'), SyndLib::translate('Search'));
	}

	function _callback_menu_commands($menu) {
		$project = $this->getActiveNode();
		while (!$project->isNull() && !($project instanceof synd_node_project))
			$project = $project->getParent();
		
		$menu->append(
			($project instanceof synd_node_project) ? tpl_link('issue','newIssue',$project->nodeId) : tpl_link('issue','newIssue'),
			SyndLib::translate('New issue'), SyndLib::translate('Accesskey: %s','N'), null, null, 'n');
		
		$menu->append("javascript:synd_ole_call('".tpl_link_call('issue','batch')."',true)", SyndLib::translate('Batch modify'));
		$menu->append("javascript:synd_ole_call('".tpl_link_call('issue','promote')."',true)", SyndLib::translate('Promote'));
		$menu->append("javascript:synd_ole_call('".tpl_link_call('issue','merge')."',true)", SyndLib::translate('Merge'));
		$menu->append("javascript:synd_ole_call('".tpl_link_call('issue','cut')."',true)", SyndLib::translate('Cut'),
			SyndLib::translate('Accesskey: %s','X'), null, null, 'x');
	}

	function _callback_index_active_clsids(&$result) {
		$result[] = 'issue';
	}

	function _callback_index_security_attributes(&$result, $user) {
		foreach ($this->getProjectOptions($user[0], 'monitor') as $project) {
			foreach ($project->getSecurityAttributes() as $attribute)
				$result[] = $attribute;
		}
		if (null != $user[0]->nodeId)
			$result[] = $user[0]->nodeId;
	}

	function _callback_breadcrumbs(&$result, Template $page) {
		$result[] = array(
			'uri' => tpl_link('issue'),
			'text' => SyndLib::translate('Projects'));

		if (null !== $this->_node && $this->_node->isPermitted('read')) {
			$crumbs = array();
			$parent = $this->_node;
			
			while (null != ($parent = $parent->getParent()) && !$parent->isNull()) {
				if (!$parent->isPermitted('read'))
					continue;
				if (($parent instanceof synd_node_project)) {
					$crumbs[] = array(
						'uri' => tpl_link('issue','project',$parent->getProjectId(),'issues'),
						'text' => $parent->toString());
				}
				else {
					$crumbs[] = array(
						'uri' => tpl_link('issue','view',$parent->nodeId),
						'text' => $parent->toString());
				}
			}
			
			$result = array_merge($result, array_reverse($crumbs));
		}
	}

	function _callback_block(&$result, Template $page) {
		require_once 'design/gui/MenuBuilder.class.inc';
		$block = $this->getTemplate();
		$menu = new ListMenu();
		$menu->append(null, SyndLib::translate('Commands'), '', array($this, '_callback_menu_commands'));
		SyndLib::runHook('issue_block', $block, $menu);

		$result[] = array(
			'text' => SyndLib::translate('Issues'),
			'content' => $block->fetch(tpl_design_path('module/issue/blocks/default.tpl'), array('menu'=>$menu)));
		
		// Render OLE blocks
		if (isset($_SESSION['synd']['issue']['ole'])) {
			foreach (array_keys($_SESSION['synd']['issue']['ole']) as $key) {
				if ($_SESSION['synd']['issue']['ole'][$key]->isCancelled())
					unset($_SESSION['synd']['issue']['ole'][$key]);
				else {
					$result[] = array('content' => 
						$page->fetchnode($_SESSION['synd']['issue']['ole'][$key],'full_view.tpl',
						array('target' => $this->getActiveNode(), 'force' => true)));
				}
			}
		}
	}

	/**
	 * Displays current issues and links to other issue listings
	 */
	function _callback_user_summary_default(&$result, Request $request, Template $page, User $user) {
		if ($this->isPermitted('read') && null != ($summary = $this->_summary($request, $page, $user, 'module/issue/user_summary_default.tpl')))
			$result .= $summary;
	}

	/**
	 * Displays links to various issue listings
	 */
	function _callback_user_summary_result(&$result, Request $request, Template $page, User $user) {
		if ($this->isPermitted('read') && null != ($summary = $this->_summary($request, $page, $user, 'module/issue/user_summary_result.tpl')))
			$result[] = $summary;
	}
	
	private function _summary($request, $page, $user, $path) {
		global $synd_user;
		$projects = $this->getProjectTree(new MethodDecider('hasPermission', array($synd_user, 'monitor')), $synd_user);
		$projects->rewind();
		
		if ($projects->valid()) {
			profiler_enter(__METHOD__);

			$summary = clone $page;
			$summary->assign('user', $user);
			$summary->assign('request', $request);

			$query = synd_node_issue::getEntityQuery(SyndNodeLib::getDefaultStorage('issue'));

			$i = $query->join('synd_issue');
			$query->in("$i.parent_node_id", new MemberIterator($projects, 'nodeId'));
			
			// Open issues
			$open = clone $query;
			$open->where("$i.client_node_id", $user->nodeId);
			$open->where("$i.info_status >= 0 AND $i.info_status < ".synd_node_issue::CLOSED);
			$summary->assign('open', $open->getEntities());
			
			// Closed issues
			$closed = clone $query;
			$closed->where("$i.client_node_id", $user->nodeId);
			$closed->where("$i.info_status >= ".synd_node_issue::CLOSED);
			$summary->assign('closed', $closed->getEntities());

			// Assigned issues
			$assigned = clone $query;
			$assigned->where("$i.assigned_node_id", $user->nodeId);
			$assigned->where("$i.info_status >= 0 AND $i.info_status < ".synd_node_issue::CLOSED);
			$summary->assign('assigned', $assigned->getEntities());

			// Projects
			$summary->assign('projects', $this->getProjects($user));
			
			profiler_leave(__METHOD__);
			return $summary->fetch($path);
		}
		
		return null;
	}

	/**
	 * Attempts to resolve issue numbers and redirect to issue
	 */
	function _callback_search_query(&$result, $query, $offset, $limit) {
		if (!$offset && preg_match('/^\s*#(\d+)\s*$/', $query, $matches) && 
			null != ($issue = synd_node_issue::findIssue(SyndNodeLib::getDefaultStorage('issue'), $matches[1])) && $issue->isPermitted('read')) {
			$result = tpl_view('issue',$issue->objectId());
			return true;
		}
	}

	/**
	 * Attempts to resolve issue numbers and display info on non-permitted
	 */
	function _callback_search_result(&$result, Request $request, Template $page, $query, $offset, $limit, $count) {
		if (!$offset && preg_match('/^\s*#?(\d+)\s*$/', $query, $matches) && 
			null != ($issue = synd_node_issue::findIssue(SyndNodeLib::getDefaultStorage('issue'), $matches[1])) && !$issue->isPermitted('read')) {
			$summary = clone $page;
			$summary->assign('issue', $issue);
			$result .= $summary->fetch(tpl_design_path('module/issue/search_result.tpl'));
		}
	}
	
	function _callback_default_request_handler(&$result) {
		if ($this->isPermitted('read')) {
			$result = $this;
			return true;
		}
	}
	
	function getDescription() {
		return SyndLib::translate('Issue tracking and project managment module.');
	}
	
	function _hook_manage_permissions(&$result) {
		$result[] = $this;	
	}
	
	function _hook_design_front(&$result) {
		global $synd_user;
		if ($synd_user->isNull())
			return;

		profiler_enter(__METHOD__);
		require_once 'design/gui/PagedListing.class.inc';
		tpl_load_stylesheet(tpl_design_uri('module/issue/issue.css'));

		$storage = SyndNodeLib::getDefaultStorage('issue');
		$database = $storage->getDatabase();

		$page = $this->getTemplate();
		$query = $database->createQuery();
		$i = $query->join('synd_issue', 'i');
		$query->column("$i.node_id");
		if (!count($order = tpl_sort_order('issue',"$i.")))
			$order = array("$i.TS_RESOLVE_BY");

		// List of newly arrived issue counts
		if (count($projects = $this->getProjectOptions($synd_user, 'manage'))) {
			$sql = "
				SELECT i.parent_node_id, COUNT(*) PROJECT_ISSUE_COUNT FROM synd_issue i
				WHERE 
					i.parent_node_id IN (".implode(', ', $database->quote(SyndLib::collect($projects, 'nodeId'))).") AND
					i.info_status >= 0 AND i.info_status < ".synd_node_issue::CLOSED." AND
					i.ts_create > ".strtotime('-1 days')." 
				GROUP BY i.parent_node_id";
			$page->assign('newProjectIssues', $database->getAssoc($sql, 0, 5));
		}
		
		// Maximal priority issues
		$projects = $this->getProjectTree(new MethodDecider('hasPermission', array($synd_user, 'manage')), $synd_user);
		$projects->rewind();
		
		if ($projects->valid()) {
			$maxprio = clone $query;
			$maxprio->in("$i.parent_node_id", new MemberIterator($projects, 'nodeId'));
			$maxprio->where("$i.assigned_node_id IS NOT NULL");
			$maxprio->where("$i.info_status >= 0 AND $i.info_status < ".synd_node_issue::CLOSED);
			$maxprio->where("$i.info_prio = ".synd_node_issue::HIGH);
			$page->assign('maxprio', new PagedListing($storage, $maxprio, $order, 25, 'maxprio'));
		}
		
		// Open issues reported by logged-in user
		$open = clone $query;
		$open->where("$i.client_node_id = ".$database->quote($synd_user->nodeId));
		$open->where("($i.assigned_node_id != ".$database->quote($synd_user->nodeId)." OR $i.assigned_node_id IS NULL)");
		$open->where("$i.info_status >= 0 AND $i.info_status < ".synd_node_issue::CLOSED);
		$page->assign('open', new PagedListing($storage, $open, $order, 50, 'open'));

		// Closed issues reported by logged-in user
		$closed = clone $query;
		$closed->where("$i.client_node_id = ".$database->quote($synd_user->nodeId));
		$closed->where("($i.assigned_node_id != ".$database->quote($synd_user->nodeId)." OR $i.assigned_node_id IS NULL)");
		$closed->where("$i.info_status >= ".synd_node_issue::CLOSED." AND $i.info_status < ".synd_node_issue::MAX_STATUS_VALUE);
		$closed->where("$i.ts_resolve > ".strtotime('-1 weeks'));
		$page->assign('closed', new PagedListing($storage, $closed, $order, 50, 'closed'));

		profiler_leave(__METHOD__);
		$result .= $page->fetch(tpl_design_path('module/issue/front.tpl'));
	}

	function _hook_field_extension(&$result, $extension) {
		$extension[0]->addFieldAlias('title', 'INFO_HEAD');
		$extension[0]->addFieldAlias('created', 'TS_CREATE');

		$extension[0]->addField('due');
		$extension[0]->addFieldAlias('due', 'TS_RESOLVE_BY');

		$extension[0]->addField('project');
		$extension[0]->addFieldAlias('project', 'PARENT_NODE_ID');

		$extension[0]->addField('assigned');
		$extension[0]->addFieldAlias('assigned', 'ASSIGNED_NODE_ID');

		$extension[0]->addFieldAlias('creator', 'client');
		$extension[0]->addFieldAlias('creator', 'CLIENT_NODE_ID');

		$extension[0]->addField('keyword');
		$extension[0]->addFieldAlias('keyword', 'category');
	}

	function _hook_field_extension_description(&$result, $field) {
		switch ($field) { 
			case 'due':
				$result .= ' '.SyndLib::translate('Defines the date when an issue is expected to be completed.');
				break;
			case 'project':
				$result .= ' '.SyndLib::translate('The project an issue or resource files under.');
				break;
			case 'assigned':
				$result .= ' '.SyndLib::translate('Person or entity primarily responsible for resolving an issue.');
				break;
			case 'client':
				$result .= ' '.SyndLib::translate('Person or entity having a stakehold in an issue.');
				break;
			case 'keyword':
				$result .= ' '.SyndLib::translate('Keyword or category an issue is filed as.');
				break;
		}
	}

	function getDefinedEvents($node) {
		return (array)Module::runHook('defined_events', array($node));
	}
	
	function getDefinedStatusCodes() {
		return $this->_config['status_codes'];
	}

	function getDefinedPriorities() {
		return $this->_config['priorities'];
	}
	
	function getProjectContextLevels() {
		return isset($this->_config['project_context_levels']) ? $this->_config['project_context_levels'] : 3;
	}
	
	function getProjectContextLimit() {
		return isset($this->_config['project_context_limit']) ? $this->_config['project_context_limit'] : 25;
	}

	/**
	 * GDPR cleanup
	 */
	function cleanup($issue) {
	        $issue->data['CREATE_NODE_ID'] = null;
        	$issue->data['UPDATE_NODE_ID'] = null;
	        $issue->data['CLIENT_NODE_ID'] = null;
        	$issue->data['ASSIGNED_NODE_ID'] = null;
	        $issue->data['INFO_CUSTOMER_HINT'] = null;
        	$issue->data['INFO_HEAD'] = 'This issue has been removed due to GDPR';
	        $issue->data['DATA_CONTENT'] = null;
        	$issue->data['DATA_EVENTLOG'] = null;

	        foreach (array_keys($issue->getFiles()) as $fid)
        	        $issue->unlink($fid);

	        $issue->save();

        	foreach ($issue->getNotes() as $note) {
                	$note->data['CREATE_NODE_ID'] = null;
	                $note->data['UPDATE_NODE_ID'] = null;
        	        $note->data['DATA_CONTENT'] = null;

                	foreach (array_keys($note->getFiles()) as $fid)
                        	$note->unlink($fid);

	                $note->save();
        	}
	}

	function runCleanup() {
		require_once 'core/lib/SyndTaskHandler.class.inc';
		$handler = SyndTaskHandler::factory('gdpr_cleanup');
		if (!$handler->isScheduled('gdpr_cleanup_task'))
			$handler->appendTask(SyndType::factory("gdpr_cleanup_task"), "gdpr_cleanup_task");
	}
	
	function _hook_defined_events(&$result, $node) {
		if (($node[0] instanceof synd_node_project)) {
			$result['oncreate'] = SyndLib::translate('Every time an issue is created');
			$result['onunassigned'] = SyndLib::translate('When an unassigned issue comes in');
			$result['onchange'] = SyndLib::translate('When an issue is changed');
			$result['onassign'] = SyndLib::translate('When an issue is assigned');
			$result['onresolve'] = SyndLib::translate('When an issue is closed');
			$result['oncancel'] = SyndLib::translate('When an issue is cancelled');
		}
		else if (($node[0] instanceof synd_node_issue)) {
			$result['onchange'] = SyndLib::translate('On each change');
			$result['onstart'] = SyndLib::translate('When work is begun');
			$result['onresolve'] = SyndLib::translate('When an issue is closed');
		}
	}

	/**
	 * Allows for syncronizing assigned issues
	 *
	 * Mounts your assigned issues as a syncable collection at
	 *  ./tasks uses VTODO
	 *  ./notes uses VJOURNAL
	 */
	function _callback_syncronize(&$result, $name, $user, $message) {
		switch (strtolower(trim($name,'./'))) {
			case 'tasks':
			case 'calendar/tasks':
				$result[] = $this->_newSyncableCollection($user[0], 'todo');
				return true;
			
			case 'notes':
			case 'calendar/notes':
				$result[] = $this->_newSyncableCollection($user[0], 'journal');
				return true;
		}
	}
	
	/**
	 * @access	private
	 */
	function _newSyncableCollection($user, $type) {
		require_once 'core/module/issue/SyncableIssueCollection.class.inc';
		$project = $this->getPrimaryProject($user);
		if ($project->isNull())
			$result = new SyncableUnauthorizedCollection();
		else {
			$sql = "
				SELECT i.node_id, i.ts_resolve_by 
				FROM synd_issue i, synd_project p
				WHERE 
					i.parent_node_id = p.node_id AND
					i.assigned_node_id = ".$project->_db->quote($user->nodeId)." AND
					i.info_status >= 0 AND i.info_status < ".synd_node_issue::CLOSED." AND
					(p.flag_hide_issues = 0 OR i.ts_resolve_by < ".time().") AND
					(i.info_prio > 0 OR i.ts_resolve_by < ".time()." OR i.info_status = ".synd_node_issue::ACTIVE.")
				ORDER BY i.ts_resolve_by";

			$collection = SyndType::factory('query_collection', $project->_storage, $sql);
			switch ($type) {
				case 'todo':
					$result = new SyncableIssueTodoCollection($collection, $project, $user);
					break;
				case 'journal':
					$result = new SyncableIssueJournalCollection($collection, $project, $user);
					break;
			}
		}
		return $result;
	}

	/**
	 * Retrives the primary project for a client, synd_node_null on failure.
	 * @param	User
	 * @return	DomainObject
	 */
	function getPrimaryProject($client) {
		global $synd_maindb, $synd_user;
		$sql = "
			SELECT p.node_id FROM synd_relation r, synd_project p
				LEFT JOIN synd_issue i
				ON (
					i.parent_node_id = p.node_id AND 
					i.ts_create > ".strtotime('-1 month')." AND
					(i.create_node_id = ".$synd_maindb->quote($synd_user->nodeId)." OR 
					 i.client_node_id = ".$synd_maindb->quote($client->nodeId)."))
			WHERE 
				r.parent_node_id = p.node_id AND
				r.child_node_id = ".$synd_maindb->quote($synd_user->nodeId)."
			GROUP BY p.node_id
			ORDER BY COUNT(i.node_id) DESC";
		if (null != ($project = SyndNodeLib::getInstance($synd_maindb->getOne($sql))))
			return $project;

		$projects = $this->getProjectOptions($client, 'read');
		if (!empty($projects))
			return $projects[key($projects)];

		return SyndNodeLib::getInstance('null.null');
	}
	
	/**
	 * Returns top-level primary projects
	 */
	function getPrimaryProjects() {
		$sql = "
			SELECT p.node_id FROM synd_project p
			WHERE p.parent_node_id = 'null.issue'";
		$storage = SyndNodeLib::getDefaultStorage('project');
		return $storage->getInstances($storage->getDatabase()->getCol($sql));
	}

	/**
	 * Returns the projects the user is a member of
	 * @param	User
	 * @return	array
	 */
	function getProjects($user) {
		$query = synd_node_project::getEntityQuery(SyndNodeLib::getDefaultStorage('project'));
		$m = $query->join('synd_project_member', 'm', false);
		$query->where("$m.parent_node_id = ".$query->getPrimaryKey());
		$query->where("$m.child_node_id", $user->nodeId);
		return $query->getEntities();
	}

	/**
	 * Returns project options
	 * @return	array
	 */
	function getProjectOptions($client, $perm = 'read') {
		profiler_enter(__METHOD__);
		$storage = SyndNodeLib::getDefaultStorage('project');
		$database = $storage->getDatabase();
		
		// Top level projects
		$sql = "
			SELECT p.node_id FROM synd_project p
			WHERE p.parent_node_id = 'null.issue'";
		$projects = $storage->getInstances($database->getCol($sql));
		foreach ($projects as $key => $project) {
			if (!$project->hasPermission($client, $perm))
				unset($projects[$key]);
		}

		// Merge member projects not included in tree from top level projects
		$sql = "
			SELECT pm.parent_node_id FROM synd_project_member pm
			WHERE pm.child_node_id = ".$database->quote($client->nodeId);
		if (count($projects))
			$sql .= " AND pm.parent_node_id NOT IN (".implode(',', $database->quote(array_keys($projects))).")";
		
		$other = $storage->getInstances($database->getCol($sql));
		foreach (array_keys($other) as $key) {
			if ($other[$key]->hasPermission($client, $perm)) {
				$parent = $other[$key]->getParent();
				while (!isset($projects[$parent->nodeId]) && !$parent->isNull()) {
					if (!$parent->hasPermission($client, $perm)) {
						$projects[$key] = $other[$key];
						continue 2;
					}
					$parent = $parent->getParent();
				}
			}
		}
		
		profiler_leave(__METHOD__);
		return $projects;
	}
	
	/**
	 * Returns the entire project tree
	 * @param	MatchDecider	Decider to specify project acceptance
	 * @param	DomainObject	User to fetch tree for
	 * @return	array
	 */
	function getProjectTree(MatchDecider $decider, DomainObject $user) {
		profiler_enter(__METHOD__);
		
		$storage = SyndNodeLib::getDefaultStorage('project');
		$query = synd_node_project::getEntityQuery($storage);
		$query->where('parent_node_id', 'null.issue');
		
		$result = new AppendIterator();
		foreach ($query->getEntities() as $project) {
			if ($decider->accept($project))
				$result->append(new RecursiveDomainObjectIterator($project, $decider));
		}
		
		foreach ($this->getProjects($user) as $project) {
			if (!$project->getParent()->isNull() && $decider->accept($project) && !$decider->accept($project->getParent()))
				$result->append(new RecursiveDomainObjectIterator($project, $decider));
		}

		profiler_leave(__METHOD__);
		return $result;
	}
	
	/**
	 * Returns the default namespace for this installation (e.g. 'example.com')
	 * @return	string	Returns null if none specified
	 */
	function getNamespace() {
		return isset($this->_config['namespace']) ? $this->_config['namespace'] : null;
	}

	function _view_index(Request $request, Template $page, $pane = null, $title = 'Projects') {
		if (!$this->isPermitted('read'))
			throw new ForbiddenException();
		$page->assign('request', $request);
		$page->assign('module', $this);
		$page->assign('pane', $pane);
		$this->_title = SyndLib::translate($title);
		return array('content' => $page->fetch(tpl_design_path('module/issue/index.tpl')));
	}
	
	function _view_issues(Request $request, Template $page)			{return $this->_view_index($request, $page, 'issues', 'My issues');}
	function _view_unassigned(Request $request, Template $page)		{return $this->_view_index($request, $page, 'unassigned', 'Unassigned issues');}
	function _view_search(Request $request, Template $page)			{return $this->_view_index($request, $page, 'search', 'Search');}

	function _view_admin(Request $request, Template $page) {
		if (!$this->isPermitted('admin'))
			throw new ForbiddenException();
		$this->_title = SyndLib::translate('Project administration');
		return array('content' => $page->fetch(tpl_design_path('module/issue/admin.tpl')));
	}

	/**
	 * Authenticates using issue tokens if need be
	 * @param	array	Module view parameters
	 * @return	array
	 */
	function _view_view(Request $request, Template $page) {
		if (isset($request[0], $request[1]) && null !== ($issue = SyndNodeLib::getInstance($request[0])) && 
			!$issue->isPermitted('read') && ($issue instanceof synd_node_issue) && $issue->validateAuthenticationToken($request[1])) {
			$page->assign('request', $request->forward()->forward());
			return array('content' => $page->fetchnode($issue,'full_view.tpl'));
		}
		return parent::_view_view($request, $page);
	}

	/**
	 * Resolves project identifier into node id
	 * @param	array	Module view parameters
	 * @return	array
	 */
	function _view_project(Request $request, Template $page) {
		if (!empty($request[0]) && null !== ($project = synd_node_project::findProject(SyndNodeLib::getDefaultStorage('project'), $request[0])))
			$request[0] = $project->nodeId;
		return parent::_view_view($request, $page);
	}

	function _view_insert(Request $request, Template $page) {
		if ('project' != $request[0]) {
			if (!isset($request['project'])) 
				throw new InvalidArgumentException();
			if (null === ($project = SyndNodeLib::getInstance($request['project'])))
				throw new NotFoundException();
			if (empty($request['node_id']) && false === ($this->_node = $project->appendChild(SyndNodeLib::factory('issue'))))
				throw new ForbiddenException();
		}
		return parent::_view_insert($request, $page);
	}

	function _view_keyword(Request $request, Template $page) {
		if (!isset($request[0]) || null === ($this->_node = SyndNodeLib::getInstance($request[0])))
			throw new NotFoundException();
		if (!$this->_node->isPermitted('read'))
			throw new ForbiddenException();
		$page->assign('request', $request->forward());
		$page->assign('node', $this->_node);
		return array('content' => $page->fetch(tpl_design_path('module/issue/keyword.tpl')));
	}

	function _view_report(Request $request, Template $page) {
		global $synd_user;
		require_once 'core/db/SyndDBLib.class.inc';
		if (!$this->isPermitted('read'))
			throw new ForbiddenException();
			
		$this->_title = SyndLib::translate('Issue report');
		$decider = new MethodDecider('hasPermission', array($synd_user, 'monitor'));

		// Project selectbox options
		$projects = $this->getProjectOptions($synd_user, 'read');
		$page->assign('projects', $projects);

		// Categories selectbox options
		$categories = array();
		$projectids = array();
		
		foreach ($this->getProjectTree($decider, $synd_user) as $project) {
			$categories = array_merge($categories, $project->getLocalCategories());
			$projectids[] = $project->nodeId;
		}
		
		$page->assign('categories', $categories);
		
		$page->assign('status_options', SyndLib::array_collect($this->getDefinedStatusCodes(), 1));
		$page->assign('priority_options', $this->getDefinedPriorities());
		
		$submit = isset($request['output']);
		if (!isset($request['status']))
			$request['status'] = '';
		if (!isset($request['report']))
			$request['report'] = 'list';
		if (!isset($request['output'])) {
			$request['subissues'] = '1';
			$request['recurse'] = '1';
		}

		if ($submit) {
			require_once 'core/module/issue/ServiceLevelReport.class.inc';
			$storage = SyndNodeLib::getDefaultStorage('issue');
			$database = $storage->getDatabase();

			$sensitive = false;
			$query = $database->createQuery();
			$i = $query->join('synd_issue', 'i');

			// Allow listing of projects with 'read' permission
			if (!empty($request['project']) && null != ($project = $storage->getInstance($request['project'])) && $project->isPermitted('read')) {
				if (empty($request['recurse']))
					$query->where("$i.parent_node_id", $project->nodeId);
				else {
					$readdecider = new MethodDecider('hasPermission', array($synd_user, 'read'));
					$query->in("$i.parent_node_id", new MemberIterator(new RecursiveDomainObjectIterator($project, $readdecider), 'nodeId'));
				}

				// If user lack 'monitor' permission, list only his/her own issues
				if (!$project->isPermitted('monitor'))
					$query->where("$i.client_node_id", $synd_user->nodeId);

				// Assigned selectbox options
				$page->assign('handlers', $project->getMembers());
			}
			else {
				if (count($projectids))
					$query->in("$i.parent_node_id", $projectids);
				else
					$query->where("$i.client_node_id", $synd_user->nodeId);
					
				// Assigned selectbox options
				$page->assign('handlers', $this->getAssignedOptions());
			}
			
			if (empty($request['subissues']))
				$query->where("$i.issue_node_id IS NULL");

			if (!empty($request['assigned'])) {
				switch ($request['assigned']) {
					case 'unassigned':
						$query->where("$i.assigned_node_id IS NULL");
						break;
					case 'assigned':
						$query->where("$i.assigned_node_id IS NOT NULL");
						break;
					default: 
						$query->where("$i.assigned_node_id = ".$database->quote($request['assigned']));
						$sensitive = true;
				}
			}
			
			switch ($request['status']) {
				case 'all':
					$query->where("$i.info_status >= 0 AND $i.info_status < ".synd_node_issue::MAX_STATUS_VALUE);
					break;
				case null:
				case 'open':
					$query->where("$i.info_status >= 0 AND $i.info_status < ".synd_node_issue::CLOSED);
					break;
				case 'closed':
					$query->where("$i.info_status >= ".synd_node_issue::CLOSED." AND $i.info_status < ".synd_node_issue::MAX_STATUS_VALUE);
					break;
				case 'overdue':
					$query->where("$i.info_status >= 0 AND $i.info_status < ".synd_node_issue::CLOSED);
					$query->where("$i.ts_resolve_by < ".time());
					break;
				default:
					$query->where("$i.info_status", $request['status']);
			}

			if (!empty($request['priority']) && is_numeric($request['priority']))
				$query->where("$i.info_prio >= {$request['priority']}");
				
			if (!empty($request['keywords']) && is_array($request['keywords'])) {
				if (empty($request['exclude_keywords'])) {
					$k = $query->join('synd_issue_keyword', 'k');
					$query->where("$k.issue_node_id = $i.node_id");
					$query->where("$k.keyword_node_id IN (".implode(',',$database->quote($request['keywords'])).")");
				}
				else {
					$query->left($i, 'synd_issue_keyword', "
						l0.issue_node_id = $i.node_id AND
						l0.keyword_node_id IN (".implode(',',$database->quote($request['keywords'])).")");
					$query->where('l0.issue_node_id IS NULL');
				}
			}

			// Specific organisation
			if (!empty($request['organisation']) && null != ($organisation = Module::getInstance('user')->resolveRole($request['organisation']))) {
				$query->in("$i.client_node_id", new MemberIterator($organisation->getChildren()->getIterator(), 'nodeId'));
				$page->assign('organisation', $organisation->getIdentifier());
			}
			
			// Specific customer
			if (!empty($request['customer'])) {
				if (null === ($user = Module::getInstance('user')->resolveEmail($request['customer'])))
					$user = SyndNodeLib::getInstance('user_case.'.$request['customer']);
				$query->where("$i.client_node_id = ".$database->quote($user->nodeId));
				$page->assign('customer', null != $user->getEmail() ? $user->getEmail() : $user->getLogin());
			}
			else if (!empty($request['customer_node_id']) && null !== ($user = SyndNodeLib::getInstance($request['customer_node_id']))) {
				$query->where("$i.client_node_id = ".$database->quote($user->nodeId));
				$page->assign('customer', null != $user->getEmail() ? $user->getEmail() : $user->getLogin());
			}

			// Created between
			if (!empty($request['created'][0]) && false != ($ts = strtotime($request['created'][0])))
				$query->where("$i.ts_create >= $ts");
			if (!empty($request['created'][1]) && false != ($ts = strtotime($request['created'][1])))
				$query->where("$i.ts_create <= $ts");

			// Updated between
			if (!empty($request['updated'][0]) && false != ($ts = strtotime($request['updated'][0])))
				$query->where("$i.ts_update >= $ts");
			if (!empty($request['updated'][1]) && false != ($ts = strtotime($request['updated'][1])))
				$query->where("$i.ts_update <= $ts");

			// Closed between
			if (!empty($request['closed'][0]) && false != ($ts = strtotime($request['closed'][0])))
				$query->where("$i.ts_resolve >= $ts");
			if (!empty($request['closed'][1]) && false != ($ts = strtotime($request['closed'][1])))
				$query->where("$i.ts_resolve <= $ts");

			// Due between
			if (!empty($request['due'][0]) && false != ($ts = strtotime($request['due'][0])))
				$query->where("$i.ts_resolve_by >= $ts");
			if (!empty($request['due'][1]) && false != ($ts = strtotime($request['due'][1])))
				$query->where("$i.ts_resolve_by <= $ts");

			// Timecard interval
			if (!empty($request['interval'][0]) && false != ($ts = strtotime($request['interval'][0]))) {
				$t = $query->join('synd_issue_task', 't');
				$query->where("$t.parent_node_id = $i.node_id");
				$query->where("$t.ts_create >= $ts");
			}
			if (!empty($request['interval'][1]) && false != ($ts = strtotime($request['interval'][1]))) {
				$t = $query->join('synd_issue_task', 't');
				$query->where("$t.parent_node_id = $i.node_id");
				$query->where("$t.ts_create <= $ts");
			}

			if (!empty($request['creator']) && null != ($user = Module::getInstance('user')->resolveEmail($request['creator'])))
				$query->where("$i.create_node_id", $user->nodeId);
			if (!empty($request['updater']) && null != ($user = Module::getInstance('user')->resolveEmail($request['updater'])))
				$query->where("$i.update_node_id", $user->nodeId);

			if (!empty($request['commenter']) && null != ($user = Module::getInstance('user')->resolveEmail($request['commenter']))) {
				$t = $query->join('synd_issue_task', 't');
				$query->where("$t.parent_node_id = $i.node_id");
				$query->where("$t.create_node_id", $user->nodeId);
			}
			
			if (null == ($order = tpl_sort_order('issue')))
				$order = array('TS_RESOLVE_BY');

			// The report will clone the query so $query is final after this
			$selector = null;
			switch ($request['report']) {
				case 'workload':
					$report = new IssueListingReport($storage, $query, $order);
					$format = 'workload';
					$name = 'Workload';
					break;

				case 'trends':
					switch ($request['format']) {
						case 'year': $format = '%Y'; break;
						case 'week': $format = '%Y-%V'; break;
						case 'day': $format = '%Y-%m-%d'; break;
						default: $format = '%Y-%m';
					}
					$report = new ServiceLevelReport($storage, $query, new ServiceLevelPeriodGrouping($format));
					$format = 'service';
					$name = 'Trends';
					break;

				case 'projects':
					$report = new ServiceLevelReport($storage, $query, new ServiceLevelProjectGrouping());
					$selector = 'project';
					$format = 'service';
					$name = 'Projects';
					break;

				case 'categories':
					$report = new ServiceLevelReport($storage, $query, new ServiceLevelCategoryGrouping());
					$format = 'service';
					$name = 'Categories';
					break;

				case 'assigned':
					$report = new ServiceLevelReport($storage, $query, new ServiceLevelAssignedGrouping());
					$selector = 'assigned';
					$format = 'service';
					$name = 'Assigned';
					break;

				case 'customers':
					$report = new ServiceLevelReport($storage, $query, new ServiceLevelCustomerGrouping());
					$selector = 'customer_node_id';
					$format = 'service';
					$name = 'Customers';
					break;

				case 'departments':
					$report = new ServiceLevelReport($storage, $query, new ServiceLevelCustomerGrouping());
					$format = 'departments';
					$name = 'Departments';
					break;

				default:
					$report = new IssueListingReport($storage, $query, $order);
					$format = 'listing';
					$name = 'Issues';
			}
			
			if ('' != $query->getWhere()) {
				if ($sensitive)
					$report->setSensitivity($sensitive);
				$page->assign('report', $report);
				$page->assign('name', $name);
				
				if (isset($request['output']['xls'])) {
					SyndLib::runHook('setlocale', LC_NUMERIC);
					header('Content-Type: application/xls');
					header('Content-Disposition: attachment; filename="'.SyndLib::translate("$name %s.xls", date('Y-m-d')).'"');
					return array('content' => $page->fetch(tpl_design_path("module/issue/report/formats/$format.xls.tpl")), 'page' => null);
				}

				$page->assign('format', $format);
				$page->assign('selector', $selector);
			}
		}
		else {
			// Assigned selectbox options
			$page->assign('handlers', $this->getAssignedOptions());
		}
		
		$page->assign('request', $request);
		return array('content' => $page->fetch(tpl_design_path('module/issue/report/report.tpl')));
	}

	function _getCollection($request) {
		if (!empty($request['selection']))
			return SyndType::factory('ole_collection', 'selection', $request['selection']);
		if (empty($request['collections']))
			return;

		if (count($request['collections']) <= 1) 
			$collection = SyndLib::getInstance(SyndLib::reset($request['collections']));
		else {
			$collection = SyndType::factory('composite_collection');
			foreach ((array)$request['collections'] as $id)
				$collection->appendCollection(SyndLib::getInstance($id));
		}
	
		return $collection;
	}

	function _view_cut(Request $request, Template $page) {
		if (null == ($collection = $this->_getCollection($request)))
			return null;

		$action = SyndType::factory('cut');
		foreach (array_keys($contents = $collection->getContents()) as $key) {
			if (!$contents[$key]->isPermitted('write'))
				throw new ForbiddenException();
			$action->appendChild($contents[$key]);
		}
		
		$_SESSION['synd']['issue']['ole'][] = $action;
	}

	/**
	 * Promotes subissues to normal issues
	 */
	function _view_promote(Request $request, Template $page) {
		if (null == ($collection = $this->_getCollection($request)))
			return null;
				
		$list = $collection->getContents();
		if (false !== array_search(false, SyndLib::invoke($list,'isPermitted','write')))
			throw new ForbiddenException();
			
		if (!empty($request['confirm'])) {
			foreach ($list as $issue) {
				$issue->getParentIssue()->removeChild($issue);
				$issue->save();
			}
		}
		else {
			$page->assign('collection', $collection);
			return array('content' => $page->fetch(tpl_design_path('module/issue/confirm_promote_issues.tpl')));
		}
	}
	
	function _view_batch(Request $request, Template $page) {
		global $synd_user;
		if (null != ($collection = $this->_getCollection($request)))
			$issues = $collection->getFilteredContents(array('synd_node_issue'));
		else
			$issues = array();
			
		if (false !== array_search(false, SyndLib::invoke($issues,'isPermitted','write')))
			throw new ForbiddenException();

		if (isset($request['post'])) {
			require_once 'core/lib/SyndDate.class.inc';
			$storage = SyndNodeLib::getDefaultStorage('project');

			$database = $storage->getDatabase();
			$database->begin();
			$result = array();

			// Skip modifications if issues are to be deleted
			if (empty($request['delete'])) {
				foreach ($issues as $issue) {
					if (!empty($request['project'])) {
						if (null == ($project = $storage->getInstance($request['project'])))
							throw new InvalidArgumentException('Specified project was not found');
						$project->appendChild($issue);
					}
					
					if (!empty($request['assigned']) && null !== ($assigned = SyndNodeLib::getInstance($request['assigned'])))
						$issue->setAssigned($assigned);
					if (!empty($request['customer']) && 
						null !== ($module = Module::getInstance('user')) &&
						null !== ($customer = $module->resolveEmail($request['customer'])))
						$issue->setCustomer($customer);

					if (!empty($request['reschedule']))
						$issue->data['TS_RESOLVE_BY'] = strtotime($request['reschedule'], $issue->data['TS_RESOLVE_BY']);
					else if (!empty($request['date']) && SyndDate::checkdate($request['date']))
						$issue->data['TS_RESOLVE_BY'] = strtotime($request['date']);

					if (isset($request['priority']) && is_numeric($request['priority']))
						$issue->setPriority($request['priority']);

					if (!empty($request['categories'])) {
						foreach ((array)$request['categories'] as $id) {
							if (null !== ($category = $storage->getInstance($id))) {
								if (!$category->isPermitted('read'))
									throw new ForbiddenException();
								if (empty($request['toggle']) || !$issue->getCategories()->exists($category))
									$issue->addCategory($category);
								else
									$issue->removeCategory($category);
							}
						}
					}

					if (isset($request['status']) && '' !== $request['status'])
						$issue->setStatus($request['status']);
					if (!empty($request['estimate']))
						$issue->setEstimate($issue->getEstimate() + $request['estimate']);

					if ($request['post'] != '0')
						$issue->save();
				}
				
				$result = SyndLib::runHook('issue_context_apply', $issues, $request);
			}
			else {
				foreach ($issues as $issue)
					$issue->delete();
			}
			
			if ($storage->flush())
				$database->commit();
			else {
				$database->rollback();
				throw new RuntimeException('Failed to commit transaction');
			}
			
			return $result;
		}
		else {
			$this->_title = SyndLib::translate('Batch modify issues');
			$page->assign('request', $request);
			$page->assign('module', $this);
			$page->assign('collection', $collection);

			$decider = new MethodDecider('hasPermission', array($synd_user, 'monitor'));

			// Project selectbox options
			$projects = $this->getProjectOptions($synd_user, 'read');
			$page->assign('projects', $projects);

			// Assigned selectbox options
			$page->assign('handlers', $this->getAssignedOptions());

			// Categories selectbox options
			$categories = array();
			foreach ($this->getProjectTree($decider, $synd_user) as $project)
				$categories = array_merge($categories, $project->getLocalCategories());
			$page->assign('categories', $categories);

			$page->assign('status_options', SyndLib::array_collect($this->getDefinedStatusCodes(), 1));
			$page->assign('priority_options', $this->getDefinedPriorities());

			return array('content' => $page->fetch(tpl_design_path('module/issue/batch.tpl')));
		}
	}
	
	/**
	 * @access	private
	 */
	function _getMergedPrimary($nid, $issues) {
		assert('!empty($issues)');
		
		if ('new' == $nid || null === ($issue = SyndNodeLib::getInstance($nid))) {
			$prototype = $issues[key($issues)];
			$project = $prototype->getParent();
			$issue = $project->appendChild($project->_storage->factory('issue'));

			// Default values for the new parent issue
			$issue->setAssigned($prototype->getAssigned());
			$issue->setCustomer($prototype->getCustomer());
			$issue->setResolveBy($prototype->getResolveBy());
			$issue->setTitle($prototype->getTitle());
			$issue->setContent(clone $prototype->getContent());
		}
		
		$issue->checkout();
		return $issue;
	}
	
	function _view_merge(Request $request, Template $page) {
		if (null == ($collection = $this->_getCollection($request)) ||
			!count($issues = $collection->getFilteredContents(array('synd_node_issue'))))
			return null;
		if (false !== array_search(false, SyndLib::invoke($issues,'isPermitted','write')))
			throw new ForbiddenException();

		$page->assign('request', $request);
		$page->assign('issues', $issues);
		$page->assign('collection', $collection);
		$this->_title = SyndLib::translate('Merge %d issues', count($issues));
		
		// Select or create primary issue
		if (empty($request[0]) || null === ($issue = $this->_getMergedPrimary($request[0], $issues)))
			return array('content' => $page->fetch(tpl_design_path('module/issue/merge/step1.tpl')));
		
		// Merge submitted data
		if (isset($request['data'])) {
			if (!count($errors = $issue->validate($request['data']))) 
				$issue->merge($request['data']);
			$page->assign('errors', $errors);
		}

		// Save and redirect if data is valid
		if (isset($request['post']) && empty($errors)) {
			$database = $issue->_storage->getDatabase();
			$database->begin();
			
			$issue->save();
			$issue->_node_onchange_internal();
			SyndLib::invoke($issue->getChildren(), 'save');
			
			if ($issue->_storage->flush())
				$database->commit();
			else
				$database->rollback();
			
			return array('redirect' => tpl_view('issue',$issue->objectId()));
		}
		
		// Set internal state to closed
		foreach ($issues as $child) {
			if ($child->nodeId != $issue->nodeId) {
				$issue->appendChild($child);
				
				if (!empty($request['close'])) {
					$child->setStatus(synd_node_issue::CLOSED);
				}

				$child->checkout();
			}
		}

		$page->assign('data', isset($request['data']) ? 
			SyndLib::array_merge_assoc($issue->getCompositeData(),$request['data']) : $issue->getCompositeData());
		$page->assign('issue', $issue);
		return array('content' => $page->fetch(tpl_design_path('module/issue/merge/step2.tpl')));
	}
	
	/**
	 * @access	private
	 * @param	issue	Issue to check if it may be reopened
	 * @return	bool	Returns true if issue may be reopened
	 */
	private function _canReopen($issue) {
		$project = $issue->getParent();
		if ($issue->isClosed() && $project->getMinimumReopenBy($issue->data['TS_RESOLVE']) < time()) {
			return false;
		}
		
		return true;
	}
	
	/**
	 * @access	private
	 * @param	array	Project argument, array(0 => 'project.123')
	 * @param	string	Raw email message
	 * @return	bool	Returns true on success, false on failure
	 */
	function _mail($request, $message) {
		global $synd_config;
		require_once 'core/lib/Mime.class.inc';

		$storage = SyndNodeLib::getDefaultStorage('issue');
		$database = $storage->getDatabase();

		// Resolve targeted project
		if (empty($request[0]))
			throw new InvalidArgumentException('No target project provided');
		if (null == ($target = $storage->getInstance($request[0])))
			throw new RuntimeException('Failed to find targeted project');

		$mime = Mime::parse($message, array($this, '_callback_verbatim_copy'));
		$email = SyndType::factory('email', $mime->getMessageText(array($this, '_callback_message_text')));
		
		if (null == ($project = $this->_findProject($storage, $target, $mime)))
			$project = $target;
		
		// Resolve sender email address
		if (null == ($sender = Module::getInstance('user')->resolveEmail($email->getSender())) &&
			null == ($sender = $this->findCustomer($project, $email->getSender())))
			$sender = SyndNodeLib::getInstance('user_case.'.$email->getSender());

		// Discard spam from senders we haven't seen before
		if ($project->isDiscardSpam() && preg_match('/^\s*Yes\b/i', $mime->getHeader('X-Spam-Status'))) {
			$sql = "
				SELECT 1 FROM synd_issue i 
				WHERE 
					i.client_node_id = ".$database->quote($sender->nodeId)." AND
					i.assigned_node_id IS NOT NULL";

			if (!$database->getOne($sql)) {
				if (SyndLib::createDirectory("/tmp/{$_SERVER['SERVER_NAME']}/discarded-issue-spam"))
					SyndLib::file_put_contents("/tmp/{$_SERVER['SERVER_NAME']}/discarded-issue-spam/".date('YmdHis').'-'.md5(uniqid('')).'.msg', $message);
				return true;
			}
		}
		
		// Detect and set locale from email language
		SyndLib::runHook('detect_locale', $email->getSubject().' '.$email->toString(), true);
		
		// Find or create issue
		if (null == ($issue = $this->_findIssue($storage, $target, $mime)) || !$this->_canReopen($issue)) {
			$issue = $storage->factory('issue');
			$issue->setParent($project);
			$issue->setCreator($sender);
			$issue->setCustomer($this->_findClient($project, $sender));

			if (($sender instanceof synd_node_case))
				$issue->setInitialSearch($email->getSender());
				
			// Resolve targeted assigned
			if (null != ($assigned = $this->_findAssigned($project, $mime)))
				$issue->setAssigned($assigned);
		
			if (null == ($title = preg_replace('/^(\s*(Fwd?|Re):\s*)+|\s*\(fwd\)|#\w+@[\w\.]+\s*$|\s+$/i', '', $email->getSubject())) &&
				null == ($title = tpl_chop($email->getBody(), 75)))
				$title = SyndLib::test('No title');

			$issue->setTitle(tpl_chop($title, 75));
			$issue->setContent($email);

			// Append files to issue
			foreach ($mime->getAttachments() as $fileinfo)
				$this->_attachment($issue, $fileinfo);

			$issue->raiseEvent(new synd_event_mail($issue));
			$issue->save();
			
			// Send confirmation email to sender
			if (!in_array(strtolower($mime->getHeader('Precedence')), array('bulk','junk','list')) && 
				!preg_match('/(postmaster|MAILER-DAEMON)@/i', $email->getSender()) && $project->getReceiptFlag()) {
				$mailNotifier = $issue->getMailNotifier();
				$mailNotifier->addTemporary($sender);
			}
		}
		else {
			// Bubble mail to parent issue if the targeted issue is closed
			while ($issue->isClosed() && null != ($parent = $issue->getParentIssue()) && !$parent->isNull())
				$issue = $parent;

			$note = $issue->appendChild($issue->_storage->factory('task'));
			$note->setDuration($issue->getParent()->isMember($sender) ? 60*20 : 0);
			$note->setCreator($sender);
			$note->setContent($email);

			// Append files to note
			foreach ($mime->getAttachments() as $fileinfo)
				$this->_attachment($note, $fileinfo);

			// Activate issue
			$issue->setStatus(synd_node_issue::RECENT);
			$issue->raiseEvent(new synd_event_mail_note($issue));
			$issue->save();

			$note->save();
		}

		return $issue->_storage->flush();
	}
	
	/**
	 * @access	private
	 */
	function _attachment($node, $fileinfo) {
		global $synd_config;
		$path = $synd_config['dirs']['cache'].md5(uniqid(''));
		if (!SyndLib::createDirectory($synd_config['dirs']['cache']) || !SyndLib::file_put_contents($path, $fileinfo['data']))
			throw new RuntimeException('Could not create temporary file');
		if (null == ($file = SyndType::factory('file', $path, $fileinfo['name']))) {
			unlink($path);
			throw new RuntimeException('Could not store file to LOB storage');
		}
		$node->appendChild($file);
		unlink($path);
		return true;
	}

	/**
	 * Email integration callback
	 *
	 * The email message including headers should be piped straight 
	 * into the request body (global $HTTP_RAW_POST_DATA). This can 
	 * be done by using a mail alias in the mailserver that pipes 
	 * the email to curl. 
	 *
	 * Postfix example (/etc/postfix/aliases) for issues@example.com, 
	 * the pipe_issue_mail.sh script handles errors and makes postfix
	 * defer delivery on errors and retry later.
	 *
	 * issues: "|/usr/local/synd/core/module/issue/pipe_issue_mail.sh http://www.example.com/synd/issue/mail/project.123/"
	 *
	 * Email having the "X-Spam-Status" header set to "Yes" will be 
	 * discarded if the system haven't seen the sender before.
	 *
	 * @param	array	Target project as array(0 => 'project.123')
	 */
	function _view_mail(Request $request, Template $page) {
		global $HTTP_RAW_POST_DATA;
		if (empty($HTTP_RAW_POST_DATA)) {
			trigger_error('No message supplied', E_USER_WARNING);
			throw new InvalidArgumentException();
		}

		//SyndLib::file_put_contents('/tmp/project.msg-'.$_SERVER['SERVER_NAME'], $HTTP_RAW_POST_DATA);
		if ($this->_mail($request, $HTTP_RAW_POST_DATA))
			return array('content' => '201', 'page' => null);
		throw new RuntimeException();
	}

	function _findProject($storage, $target, $mime) {
		// Match headers for '+projectid@'
		foreach (array('Delivered-To','X-Original-To','To','Cc','Resent-To') as $header) {
			if (preg_match_all('/\+([\w\-]*?[a-z][\w\-]*)[\+@]/i', $mime->getHeader($header), $matches)) {
				foreach ($matches[1] as $match) {
					if (null != ($project = synd_node_project::findProject($storage, $match))) {
						for ($parent = $project; !$parent->isNull(); $parent = $parent->getParent()) {
							if ($parent->nodeId == $target->nodeId)
								return $project;
						}
					}
				}
			}
		}
		
		// Match subject for #projectid@example.com
		if (null != $this->getNamespace()) {
			$pattern = '/#([\w\-]*?[a-z][\w\-]*)@'.preg_quote(preg_replace('/\s/', '', $this->getNamespace()), '/').'/i';
		
			if (preg_match_all($pattern, preg_replace('/\s/', '', $mime->getHeader('Subject')), $matches)) {
				foreach ($matches[1] as $match) {
					if (null != ($project = synd_node_project::findProject($storage, $match)) && $this->_isProjectChild($target, $project))
						return $project;
				}
			}
		}
		
		return null;
	}

	protected function _findIssue($storage, $project, $mime) {
		// Match headers for someproject+12345@example.com
		foreach (array('Delivered-To','X-Original-To','To','Cc','Resent-To') as $header) {
			if (preg_match_all('/[+.](\d+)@/', $mime->getHeader($header), $matches)) {
				foreach ($matches[1] as $match) {
					if (null != ($issue = synd_node_issue::findIssue($storage, $match)) && $this->_isProjectChild($project, $issue))
						return $issue;
				}
			}
		}
		
		// Match subject for #12345@example.com
		if (null != $this->getNamespace()) {
			$pattern = '/#(\d+)@'.preg_quote(preg_replace('/\s/', '', $this->getNamespace()), '/').'/i';
		
			if (preg_match_all($pattern, preg_replace('/\s/', '', $mime->getHeader('Subject')), $matches)) {
				foreach ($matches[1] as $match) {
					if (null != ($issue = synd_node_issue::findIssue($storage, $match)) && $this->_isProjectChild($project, $issue))
						return $issue;
				}
			}
		}

		return null;
	}
	
	protected function _isProjectChild($project, $issue) {
		for ($parent = $issue->getParent(); !$parent->isNull(); $parent = $parent->getParent()) {
			if ($parent->nodeId == $project->nodeId)
				return true;
		}
		return false;
	}
	
	/**
	 * @access	private
	 */
	function _findAssigned($project, $mime) {
		$logins = array();
		$users = array();
		
		foreach ($project->getMembers() as $key => $user) {
			$logins[$key] = strtolower($user->getLogin());
			$users[$key] = $user;
		}
		
		// Match headers for '+username@'
		foreach (array('Delivered-To','X-Original-To','To','Cc','Resent-To') as $header) {
			if (preg_match_all('/\+([\w\-]*?[a-z][\w\-]*)[\+@]/i', $mime->getHeader($header), $matches)) {
				foreach ($matches[1] as $match) {
					if (false !== ($key = array_search(strtolower($match), $logins)))
						return $users[$key];
				}
			}
		}
		
		return null;
	}

	function findCustomer($project, $query) {
		$ids = array();
		for ($parent = $project; !$parent->isNull(); $parent = $parent->getParent()) 
			$ids[] = $parent->nodeId;
		
		if (!empty($ids)) {
			$sql = "
				SELECT pm.customer_node_id FROM synd_project_mapping pm
				WHERE 
					pm.project_node_id IN (".implode(',', $project->_db->quote($ids)).") AND
					pm.query = ".$project->_db->quote($query);
			return SyndLib::getInstance($project->_db->getOne($sql));
		}
		
		return null;
	}
	
	/**
	 * @access	private
	 */
	function _findClient($project, $sender) {
		$default = $project->getDefaultClient();
		if (!$default->isNull())
			return $default;
		return $sender;
	}
	
	function _callback_verbatim_copy($mime) {
		return SyndLib::runHook('issue_verbatim_copy', $mime);
	}
	
	function _callback_message_text($mime) {
		return SyndLib::runHook('issue_message_text', $mime);
	}

	/**
	 * Autosuggest clients callback
	 * @see		synd_module_user::findSuggestedUsers()
	 */
	function findSuggestedUsers($query) {
		$module = Module::getInstance('user');
		$suggestions = $module->findSuggestedUsers($query);
		$suggestions["\"$query\""] = SyndLib::translate('Text <em>"%s"</em> only', $query);
		return $suggestions;
	}

	/**
	 * Autosuggest clients callback
	 * @see		synd_module_user::findSuggestedUsers()
	 */
	function _public_findSuggestedUsers($query) {
		if (!$this->isPermitted('read'))
			throw new ForbiddenException();
		ignore_user_abort(false);
		return $this->findSuggestedUsers($query);
	}

	/**
	 * Autosuggest roles callback
	 * @see		synd_module_user::findSuggestedRoles()
	 */
	function _public_findSuggestedRoles($query) {
		if (!$this->isPermitted('read'))
			throw new ForbiddenException();
		ignore_user_abort(false);
		return Module::getInstance('user')->findSuggestedRoles($query);
	}

	function getAssignedOptions() {
		global $synd_user;
		$users = array();
		foreach ($this->getProjectTree(new MethodDecider('hasPermission', array($synd_user, 'monitor')), $synd_user) as $project) {
			foreach ($project->getMembers() as $user)
				$users[$user->nodeId] = $user;
		}
		return $users;
	}
	
	function _public_getAssignedOptions() {
		if (!$this->isPermitted('read'))
			throw new ForbiddenException();
		$options = array();
		foreach (array_keys($assigned = $this->getAssignedOptions()) as $key)
			$options[$assigned[$key]->nodeId] = $assigned[$key]->toString().' ('.$assigned[$key]->getContact().')';
		uasort($options, 'strcasecmp');
		return $options;
	}

	function _view_newProject(Request $request, Template $page) {
		if (!$this->isPermitted('admin'))
			throw new ForbiddenException();
		$project = SyndNodeLib::factory('project');
		return array('redirect' => tpl_view('issue','edit',$project->nodeId));
	}
	
	function _view_newIssue(Request $request, Template $page) {
		global $synd_user;
		if (!$this->isPermitted('read'))
			throw new ForbiddenException();

		$issue = SyndNodeLib::factory('issue');
		if (empty($request[0]) || null == ($project = SyndNodeLib::getInstance($request[0]))) 
			$project = $this->getPrimaryProject($synd_user);

		$project->appendChild($issue);
		if (array_key_exists($synd_user->nodeId, $issue->getAssignedOptions()))
			$issue->setAssigned($synd_user);
		if (isset($request['customer']) && null !== ($customer = SyndNodeLib::getInstance($request['customer'])))
			$issue->setCustomer($customer);

		return array('redirect' => tpl_view('issue','edit',$issue->nodeId));
	}
}
