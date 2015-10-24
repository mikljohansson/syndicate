<?php
require_once 'core/model/DatabaseEntity.class.inc';
require_once 'core/model/DomainEvent.class.inc';
require_once 'core/Module.class.inc';

class synd_node_issue extends AbstractDatabaseEntity {
	const PENDING			= 0;	// Pending action status range 0-4
	const RECENT			= 2;	// Status level is handled automatically and counts as a variant of PENDING
	const ACTIVE			= 5;	// Work in progress status range 5-9
	const CLOSED			= 10;	// Closed or closed status = 10
	const MAX_STATUS_VALUE	= 20;	// Max status value, used for database index performance reasons

	const LOW				= 0;
	const NORMAL			= 1;
	const HIGH				= 2;

	/**
	 * @access	private
	 */
	var $_attributes = null;

	/**
	 * @access	private
	 */
	var $_task = null;
	
	/**
	 * @var		AggregateEntityCollection
	 */
	protected $_notes = null;

	/**
	 * @var		AggregateEntityCollection
	 */
	protected $_issues = null;
	protected $_open = null;
	
	/**
	 * @access	private
	 */
	var $_mailNotifier = null;

	/**
	 * @access	private
	 */
	var $_eventLogger = null;

	/**
	 * Issue categories
	 * @var		AssociateEntityCollection
	 */
	protected $_categories = null;
	
	/**
	 * @access	private
	 */
	var $_content = null;
	
	/**
	 * @access	private
	 */
	var $_excerpt = null;
	
	function __construct($storage, $database, $data) {
		if (null != $data['DATA_FILES'])
			$data['DATA_FILES'] = unserialize($data['DATA_FILES']);
		parent::__construct($storage, $database, $data);

		$this->_storage->preload($this->data['PARENT_NODE_ID']);
		SyndNodeLib::preload($this->data['CREATE_NODE_ID']);
		SyndNodeLib::preload($this->data['CLIENT_NODE_ID']);
		SyndNodeLib::preload($this->data['ASSIGNED_NODE_ID']);
		
		$this->_notes = $this->aggregate('synd_node_task');
		$this->_issues = $this->aggregate('synd_node_issue', 'ISSUE_NODE_ID');
		$this->_categories = $this->associate(
			'synd_issue_keyword', 'synd_node_keyword', 'ISSUE_NODE_ID', 'KEYWORD_NODE_ID', 
			array('synd_node_issue','_callback_categories_filter'));
	}
	
	function __factory() {
		global $synd_user;

		$id = $this->_db->nextId('synd_issue');
		$this->data['NODE_ID'] = $this->classId().'.'.$id;
		$this->data['INFO_ISSUE_ID'] = $id;
		
		$this->data['TS_CREATE'] = $this->data['TS_UPDATE'] = time();
		$this->data['CREATE_NODE_ID'] = $this->data['UPDATE_NODE_ID'] = $synd_user->nodeId;
		$this->data['INFO_PRIVATE_KEY'] = md5(uniqid($this->nodeId));
		
		parent::__factory();
	}

	function __clone() {
		global $synd_user;
		$notes = $this->getNotes()->getIterator();
		$issues = $this->getChildren()->getIterator();
		$categories = $this->getCategories()->getIterator();
		
		$id = $this->_db->nextId('synd_issue');
		$this->data['NODE_ID'] = $this->classId().'.'.$id;
		$this->data['INFO_ISSUE_ID'] = $id;

		$this->data['TS_CREATE'] = $this->data['TS_UPDATE'] = time();
		$this->data['CREATE_NODE_ID'] = $this->data['UPDATE_NODE_ID'] = $synd_user->nodeId;
		$this->data['INFO_PRIVATE_KEY'] = md5(uniqid($this->nodeId));
		
		parent::__clone();
		
		$this->_notes = $this->aggregate('synd_node_task');
		$this->_issues = $this->aggregate('synd_node_issue', 'ISSUE_NODE_ID');
		$this->_categories = $this->associate('synd_issue_keyword', 'synd_node_keyword', 'ISSUE_NODE_ID', 'KEYWORD_NODE_ID');

		foreach ($notes as $note)
			$this->appendChild(clone $note);
		foreach ($issues as $issue)
			$this->appendChild(clone $issue);
		foreach ($categories as $category)
			$this->_categories->append($category);
			
		$this->setContent(clone $this->getContent());
	}
	
	function __sleep() {
		$vars = parent::__sleep();

		if (null !== $this->_eventLogger)
			$this->data['DATA_EVENTLOG'] = serialize($this->_eventLogger);
		unset($vars[array_search('_eventLogger', $vars)]);

		if (null !== $this->_content)
			$this->data['DATA_CONTENT'] = serialize($this->_content);
		unset($vars[array_search('_content', $vars)]);

		return $vars;
	}
	
	function __wakeup() {
		parent::__wakeup();
		$this->_storage->preload($this->data['PARENT_NODE_ID']);
		SyndNodeLib::preload($this->data['CREATE_NODE_ID']);
		SyndNodeLib::preload($this->data['CLIENT_NODE_ID']);
		SyndNodeLib::preload($this->data['ASSIGNED_NODE_ID']);
	}
	
	static function getEntityQuery(DomainStorage $storage) {
		return self::createQuery($storage, 'synd_issue');
	}

	function nodeId() {
		return $this->data['NODE_ID'];
	}

	function getHandler() {
		return 'issue';
	}

	function hasPermission($user, $perm, $arg = null) {
		if ('read' == $perm && null != $arg)
			return 'full_view_email.tpl' == $arg && $this->hasPermission($user, 'write') || $this->validateAuthenticationToken($arg);
		
		// Read permission must not bubble to project as many people have 
		// project read permission but not all should see all issues
		if ('read' == $perm) {
			if ($this->getAssigned()->hasAuthorization($user, $this) || $this->hasPermission($user, 'write') || 
				$this->getCustomer()->hasAuthorization($user, $this) || $this->getCreator()->hasAuthorization($user, $this))
				return true;
			
			// Escalate 'read' to 'write' on parent issue if one exists
			$issue = $this->getParentIssue();
			if ($issue->hasPermission($user, 'write'))
				return true;

			return parent::hasPermission($user, 'monitor', $arg);
		}

		if ('write' == $perm && (
			$this->getAssigned()->hasAuthorization($user, $this) ||
			$this->getCustomer()->hasAuthorization($user, $this) || 
			$this->getCreator()->hasAuthorization($user, $this)))
			return true;

		// Performance improvement
		if ('write' == $perm && null != ($updated = SyndNodeLib::getInstance($this->data['UPDATE_NODE_ID'])) &&
			$updated->hasAuthorization($user, $this))
			return true;

		// Change status (active, waiting, ...), assign to user, add/remove categories
		if ('status' == $perm || 'assign' == $perm || 'categorize' == $perm)
			return $this->getParent()->hasPermission($user, 'write') || $this->getAssigned()->hasAuthorization($user, $this);

		if ('admin' == $perm && $this->getParent()->hasPermission($user, 'write'))
			return true;

		// Mail callback and syncable needs to append without proper authorization
		if ('append' == $perm) {
			if (($arg instanceof synd_node_task) /*&& $this->hasPermission($user, 'write')*/)
				return true;
			if (($arg instanceof synd_node_issue) && $this->hasPermission($user, 'write')) {
				for ($parent = $this; !$parent->isNull(); $parent = $parent->getParentIssue()) {
					if ($parent->nodeId == $arg->nodeId)
						return false;
				}
			}
			return true;
		}
		
		if ('remove' == $perm)
			return (($arg instanceof synd_node_task) || ($arg instanceof synd_node_issue)) && $this->hasPermission($user, 'write');

		// Set the client of this issue
		if ('issue_client' == $perm)
			return $this->getParent()->hasPermission($user, 'write');
			
		// Edit all notes
		if ('issue_task_write' == $perm)
			return $this->getParent()->hasPermission($user, 'write');
				
		if ('merge' == $perm && in_array($arg, array(
			'PARENT_NODE_ID','ASSIGNED_NODE_ID','CLIENT_NODE_ID','TS_RESOLVE_BY',
			'INFO_HEAD','INFO_ESTIMATE')))
			return $this->hasPermission($user, 'write');
			
		if ('merge' == $perm && in_array($arg, array('INFO_PRIO')))
			return $this->hasPermission($user, 'admin');

		return parent::hasPermission($user, $perm, $arg);
	}

	function getParent() {
		if (null === ($parent = $this->_storage->getInstance($this->data['PARENT_NODE_ID'])))
			$parent = parent::getParent();
		return $parent;
	}

	function setParent($parent) {
		if ($this->data['PARENT_NODE_ID'] != $parent->nodeId) {
			$this->data['PARENT_NODE_ID'] = $parent->nodeId;
			if (self::PENDING == $this->data['INFO_STATUS'])
				$this->setStatus(self::RECENT);
			if (!$this->isNew())
				$this->fireEvent(SyndType::factory('onsetparent', $this));
		}
	}
	
	function getAttribute($key) {
		if (null === $this->_attributes && null != $this->data['DATA_ATTRIBUTES'])
			$this->_attributes = unserialize($this->data['DATA_ATTRIBUTES']);
		return isset($this->_attributes[$key]) ? $this->_attributes[$key] : parent::getAttribute($key);
	}
	
	function setAttribute($key, $value) {
		if (null === $this->_attributes && null != $this->data['DATA_ATTRIBUTES'])
			$this->_attributes = unserialize($this->data['DATA_ATTRIBUTES']);
		if (null != $value)
			$this->_attributes[$key] = $value;
		else if (isset($this->_attributes[$key]))
			unset($this->_attributes[$key]);
		$this->_node_onchange_internal();
	}
	
	/**
	 * Set the assigned user. Fires an 'onassign' event.
	 * @see		synd_type_onassign
	 * @param	User
	 */
	function setAssigned($assigned) {
		global $synd_user;
		if ($this->data['ASSIGNED_NODE_ID'] != $assigned->nodeId) {
			$this->data['ASSIGNED_NODE_ID'] = $assigned->nodeId;

			if (self::PENDING == $this->data['INFO_STATUS'])
				$this->setStatus(self::RECENT);

			if (!$this->isNew()) {
				if ($assigned->isNull())
					$this->fireEvent(SyndType::factory('onunassigned', $this));
				else
					$this->fireEvent(SyndType::factory('onassign', $this));
			}
		}
	}
	
	/**
	 * Returns user assigned this issue
	 * @return	User
	 */
	function getAssigned() {
		if (null === ($assigned = SyndNodeLib::getInstance($this->data['ASSIGNED_NODE_ID'])))
			$assigned = SyndNodeLib::getInstance('user_null.null');
		return $assigned;
	}
	
	/**
	 * Returns the users possible to assign this issue to
	 * @return	array
	 */
	function getAssignedOptions() {
		$options = iterator_to_array($this->getParent()->getMembers()->getIterator());
		$assigned = $this->getAssigned();
		if (!$assigned->isNull() && !isset($options[$assigned->nodeId]))
			$options[$assigned->nodeId] = $assigned;
		return $options;
	}

	/**
	 * Returns projects possible to append this issue to
	 * @return	array
	 */
	function getParentOptions() {
		$parent = $this->getParent();
		if ($parent->isNull()) 
			return array();
		// Merge current project if not already included
		$projects = $parent->getParentOptions();
		if (!isset($projects[$parent->nodeId]) && !$parent->isNull() && !$parent->isPermitted('read'))
			$projects[$parent->nodeId] = $parent;
		return $projects;
	}

	/**
	 * Returns the keywords assigned to this issue
	 * @return	array
	 */
	function getCategories() {
		return $this->_categories;
	}
	
	/**
	 * Assigns a category to this issue
	 * @param	synd_node_keyword
	 * @return	bool	Returns true if the category was actually added
	 */
	function addCategory($category) {
		return $this->_categories->append($category);
	}

	/**
	 * Removes a category from this issue
	 * @param	synd_node_keyword
	 */
	function removeCategory($category) {
		return $this->_categories->remove($category);
	}

	/**
	 * Returns the first two keywords rendered like "[Keyword1] [Keyword2]"
	 * @access	protected
	 * @return	string
	 */
	function getTitleCategories() {
		if (count($this->_categories))
			return '['.implode('] [', SyndLib::invoke($this->_categories->getIterator(0, 2), 'toString')).']';
		return null;
	}

	/**
	 * Returns most recent/relevant keywords for the current user
	 * @access	protected
	 * @param	integer	Number of keywords to return
	 * @return	array
	 */
	function getRecentCategories($options, $limit = 6) {
		global $synd_user;
		if (!count($options) || $limit <= 0)
			return array();
		$ids = SyndLib::collect($options, 'nodeId');
		
		$sql = "
			SELECT ik.keyword_node_id, SUM(POWER(0.999999,".time()."-ik.ts_create)) weight 
			FROM synd_issue_keyword ik
			WHERE 
				ik.keyword_node_id IN (".implode(', ', $this->_db->quote($ids)).") AND
				ik.create_node_id = ".$this->_db->quote($synd_user->nodeId)." AND
				ik.ts_create > ".strtotime('-1 month')."
			GROUP BY ik.keyword_node_id
			ORDER BY weight DESC";

		$result = $this->_db->getCol($sql,0,0,$limit);
		$ids = array_diff($ids, $result);
		
		if (count($result) < $limit && count($result) < count($ids)) {
			$sql = "
				SELECT ik.keyword_node_id, SUM(POWER(0.999999,".time()."-ik.ts_create)) weight 
				FROM synd_issue_keyword ik
				WHERE 
					ik.keyword_node_id IN (".implode(', ', $this->_db->quote(array_diff($ids, $result))).") AND
					ik.ts_create > ".strtotime('-1 month')."
				GROUP BY ik.keyword_node_id
				ORDER BY weight DESC";

			$result = array_merge($result, $this->_db->getCol($sql,0,0,$limit-count($result)));
			$ids = array_diff($ids, $result);
		}
			
		return $this->_storage->getInstances($result);
	}

	static function _callback_categories_filter($row) {
		global $synd_user;
		$row['TS_CREATE'] = time();
		$row['CREATE_NODE_ID'] = $synd_user->nodeId;
		return $row;
	}
	
	/**
	 * Creates a context menu option
	 * @param	string	Key to set
	 * @param	string	Value of option
	 * @param	string	User visible text
	 * @param	mixed	Id or list of ids to select (string for single-select, array for multiple select)
	 * @param	string	Endpoint to load submenu from, used for multiple submenu levels
	 * @param	bool	Save to database
	 */
	static function newContextOption($key, $value, $caption, $selected = null, $endpoint = null, $save = true) {
		if (is_array($selected) && !isset($selected[$value]) || !is_array($selected) && $selected != $value)
			$result = array($caption);
		else if (is_array($selected))
			$result = array('<img src="'.tpl_design_uri('image/icon/check.gif').'" />'.$caption);
		else 
			$result = array('<img src="'.tpl_design_uri('image/icon/radio.gif').'" />'.$caption);

		$result[] = tpl_view('issue','batch',array('post'=>(int)$save));
		$result[] = $key;
		$result[] = $value;
		$result[] = $endpoint;

		return $result;
	}

	static function treeContextOptions(&$result, $projects, $selected, $pad = 0) {
		$levels = Module::getInstance('issue')->getProjectContextLevels();
		foreach ($projects as $project) {
			$children = iterator_to_array(new MethodFilterIterator($project->getPublishedProjects()->getIterator(), 'isPermitted', 'read'));
			$result[] = self::newContextOption(
				'project', $project->nodeId, str_repeat('&nbsp;&nbsp;&nbsp;',$pad).$project->toString(), $selected, 
				$pad >= $levels-1 && !empty($children) ? array(tpl_request_host().tpl_view('rpc','json',$project->id()), 'getContextMenu', array($selected)) : null);
			if ($pad < $levels-1 && !empty($children))
				self::treeContextOptions($result, SyndLib::sort($children), $selected, $pad+1);
		}
	}

	static function treeCategoryRenderOptions(&$result, $categories, $selected, $levels, $pad = 0) {
		$count = 0;

		foreach ($categories as $category) {
			$children = iterator_to_array($category->getCategories()->getIterator());
			
			// Endpoint from where to fetch submenu
			$endpoint = null; 
			if ($pad >= $levels-1 && !empty($children))
				$endpoint = array(tpl_request_host().tpl_view('rpc','json',$category->id()), 'getContextMenu', array($selected));
			
			$result[] = self::newContextOption(
				'categories', $category->isSelectable() ? $category->nodeId : null, 
				str_repeat('&nbsp;&nbsp;&nbsp;',$pad).$category->toString(), 
				$selected, $endpoint);
			
			if ($pad < $levels-1 && !empty($children))
				$count += self::treeCategoryRenderOptions($result, SyndLib::sort($children), $selected, $pad+1);
			$count++;
		}

		return $count;
	}
	
	static function treeCategoryContextOptions(&$result, $categories, $selected) {
		$levels = Module::getInstance('issue')->getProjectContextLevels();
		$max = Module::getInstance('issue')->getProjectContextLimit();
		$count = 0;
		$options = array();

		do {
			$options = array();
			$count = self::treeCategoryRenderOptions($options, $categories, $selected, $levels);
		} while ($count > $max && $levels-- > 1);

		foreach ($options as $option) {
			$result[] = $option;
		}
	}

	/**
	 * Builds the context menus
	 * @param	bool	Display workload menus
	 * @return	array
	 */
	function _public_getContextMenus($workload = false) {
		$project = $this->getParent();
		$result = array(
			'assigned'		=> null,
			'project'		=> null,
			'categories'	=> null,
			'priority'		=> null,
			'status'		=> null,
			'reschedule'	=> null,
			'estimate'		=> null,
			);

		// Assigned context menu
		if ($this->isPermitted('assign')) {
			$assigned = $this->getAssigned();
			$result['assigned'][] = self::newContextOption('assigned', 'user_null.null', SyndLib::translate('Unassigned'), $assigned->nodeId());
			foreach (array_keys($options = SyndLib::sort($this->getAssignedOptions())) as $key)
				$result['assigned'][] = self::newContextOption('assigned', $options[$key]->nodeId(), $options[$key]->toString(), $assigned->nodeId());
		}

		// Project context menu
		if ($this->isPermitted('write'))
			self::treeContextOptions($result['project'], SyndLib::sort($this->getParentOptions()), $project->nodeId);

		// Categories context menu
		if ($this->isPermitted('categorize')) {
			$categories = $project->getCategories();
			$selected = iterator_to_array($this->getCategories());

			if (!empty($categories)) {
				self::treeCategoryContextOptions($result['categories'], SyndLib::sort($categories), $selected);
			}
		}

		if ($this->isPermitted('write')) {
			// Priority context menu
			foreach ($this->getDefinedPriorities() as $key => $value)
				$result['priority'][] = self::newContextOption('priority', $key, SyndLib::translate($value), $this->data['INFO_PRIO']);

			// Status context menu
			foreach ($this->getDefinedStatusCodes() as $key => $value)
				$result['status'][] = self::newContextOption('status', $key, SyndLib::translate($value[1]), $this->data['INFO_STATUS']);

			// Reschedule context menu
			$result['reschedule'][] = self::newContextOption('reschedule', '+12 weeks', SyndLib::translate('+%d months',3));
			$result['reschedule'][] = self::newContextOption('reschedule', '+4 weeks', SyndLib::translate('+%d weeks',4));
			$result['reschedule'][] = self::newContextOption('reschedule', '+2 weeks', SyndLib::translate('+%d weeks',2));
			$result['reschedule'][] = self::newContextOption('reschedule', '+1 weeks', SyndLib::translate('+%d week',1));
			$result['reschedule'][] = self::newContextOption('reschedule', '-1 weeks', SyndLib::translate('-%d week',1));
			$result['reschedule'][] = self::newContextOption('reschedule', '-2 weeks', SyndLib::translate('-%d weeks',2));
			$result['reschedule'][] = self::newContextOption('reschedule', '-4 weeks', SyndLib::translate('-%d weeks',4));
			$result['reschedule'][] = self::newContextOption('reschedule', '-12 weeks', SyndLib::translate('-%d months',3));

			// Estimate context menu
			$result['estimate'][] = self::newContextOption('estimate', 3600*8, SyndLib::translate('+%d hours',8));
			$result['estimate'][] = self::newContextOption('estimate', 3600*4, SyndLib::translate('+%d hours',4));
			$result['estimate'][] = self::newContextOption('estimate', 3600*2, SyndLib::translate('+%d hours',2));
			$result['estimate'][] = self::newContextOption('estimate', 3600, SyndLib::translate('+%d hour',1));
			$result['estimate'][] = self::newContextOption('estimate', -3600, SyndLib::translate('-%d hour',1));
			$result['estimate'][] = self::newContextOption('estimate', -3600*2, SyndLib::translate('-%d hours',2));
			$result['estimate'][] = self::newContextOption('estimate', -3600*4, SyndLib::translate('-%d hours',4));
			$result['estimate'][] = self::newContextOption('estimate', -3600*8, SyndLib::translate('-%d hours',8));
			
			SyndLib::runHook('issue_context_menu', $this, array(&$result));
		}

		return $result;
	}
	
	/**
	 * Returns the parent issue
	 * @return	DomainObject
	 */
	function getParentIssue() {
		if (null === ($issue = $this->_storage->getInstance($this->data['ISSUE_NODE_ID'])))
			$issue = SyndNodeLib::getInstance('null.null');
		return $issue;
	}

	/**
	 * Returns child issues
	 * @return	DomainObjectCollection
	 */
	function getChildren() {
		return $this->_issues;
	}
	
	/**
	 * Returns open child issue count
	 * @return	integer	Returns null if no child issues exist
	 */
	function getOpenCount() {
		if (null === $this->_open) {
			profiler_enter(__METHOD__);
			$sql = "
				SELECT COUNT(1) TOTAL, COUNT(i.ts_resolve) RESOLVED FROM synd_issue i
				WHERE i.issue_node_id = ".$this->_db->quote($this->nodeId);
			$this->_open = null != ($row = $this->_db->getRow($sql)) && !empty($row['TOTAL']) ? $row['TOTAL'] - $row['RESOLVED'] : '';
			$this->_node_onchange_internal();
			profiler_leave(__METHOD__);
		}
		return $this->_open;
	}

	/**
	 * Returns a unix timestamp saying when issue should be resolved.
	 * @return	integer
	 */
	function getResolveBy() {
		return $this->data['TS_RESOLVE_BY'];
	}
	
	/**
	 * Sets the resolve by date
	 * @param	integer	Unix timestamp
	 */
	function setResolveBy($resolve) {
		if ($this->data['TS_RESOLVE_BY'] != $resolve)
			$this->data['TS_RESOLVE_BY'] = $resolve;
	}

	function getDefinedStatusCodes() {
		return Module::getInstance('issue')->getDefinedStatusCodes();
	}
	
	/**
	 * Transition issue status
	 * @param	integer	Status value to set
	 * @param	integer	Optional timestamp of status change, defaults to current system time
	 */
	function setStatus($status, $timestamp = null) {
		if ($this->data['INFO_STATUS'] != $status) {
			if ($status >= self::CLOSED) {
				$this->data['TS_RESOLVE'] = null != $timestamp ? $timestamp : time();

				// Activate parent issue
				if (!$this->getParentIssue()->isNull())
					$this->getParentIssue()->setStatus(self::ACTIVE);

				$this->fireEvent(SyndType::factory('onresolve', $this));
				$this->_node_onchange_internal();
			}
			else {
				$this->data['TS_RESOLVE'] = null;
				if ($status >= self::ACTIVE) {
					$this->data['TS_START'] = null != $timestamp ? $timestamp : time();
					$this->fireEvent(SyndType::factory('onstart',$this));
				}
				else if ($status >= self::RECENT) {

				}
				else if ($status >= self::PENDING) {
					$this->data['TS_START'] = null != $timestamp ? $timestamp : time();
					$this->fireEvent(SyndType::factory('onpause',$this));
				}
			}

			$this->data['INFO_STATUS'] = $status;
		}
	}
	
	function getStatus() {
		return $this->data['INFO_STATUS'];
	}

	/**
	 * Returns the status of this issue in {Recent, Pending, Active, Closed}
	 * @return	string
	 */
	function getStatusName() {
		$codes = $this->getDefinedStatusCodes();
		return isset($codes[$this->data['INFO_STATUS']]) ? $codes[$this->data['INFO_STATUS']][0] : '';
	}

	/**
	 * Indicates if issue has been closed
	 * @return	bool
	 */
	function isClosed() {
		return $this->data['INFO_STATUS'] >= self::CLOSED;
	}
	
	/**
	 * Indicates if this issue cas passed its due date
	 * @return	bool
	 */
	function isOverdue() {
		return !$this->isClosed() && $this->data['TS_RESOLVE_BY'] < time()-3600*24;
	}
	
	/**
	 * Returns the priority of this issue in {Low, Normal, High}
	 * @return	string
	 */
	function getPriorityName() {
		switch ($this->data['INFO_PRIO']) {
			case 0:
				return 'Low';
			case 2:
				return 'High';
			default:
				return 'Normal';
		}
	}

	function save() {
		parent::save();

		if (null !== ($task = $this->_storage->getInstance($this->_task))) {
			$mailNotifier = $this->getMailNotifier();
			$emails = array();

			foreach ((array)$mailNotifier->getTemporaryUsers() as $user)			
				$emails[] = $this->_formatEmailSender($user);
			foreach ((array)$mailNotifier->getTemporaryEmails() as $email)
				$emails[] = $email;
			
			// Convert plaintext note to email
			if (!empty($emails)) {
				require_once 'core/lib/Mime.class.inc';
				$mime = Mime::createTextpart($task->getDescription());
				if (count($senders = $this->getEmailSenders()) > 1) 
					$mime->setHeader('Sender', reset($senders));
				$mime->setHeader('From', implode(', ', $senders));
				$mime->setHeader('To', array_shift($emails));
				if (!empty($emails))
					$mime->setHeader('Cc', implode(', ', $emails));
				$task->setContent(SyndType::factory('email', $mime));
			}

			$task->save();
			$this->_task = null;
		}

		if ($this->isNew())
			$this->fireEvent(SyndType::factory('oncreate', $this));
		else
			$this->fireEvent(SyndType::factory('onchange', $this));
	}
	
	/**
	 * Adds client security attribute
	 */
	function getSecurityAttributes() {
		$attributes = parent::getSecurityAttributes();
		if (null != $this->data['CLIENT_NODE_ID'])
			$attributes[] = $this->data['CLIENT_NODE_ID'];
		return $attributes;
	}
	
	function acceptIndexer(IndexBuilder $indexer) {
		parent::acceptIndexer($indexer);
		
		$indexer->visitFragment($this->objectId(), 'id');
		$indexer->visitFragment($this->data['INFO_HEAD'], 'title');

		$this->getContent()->acceptIndexer($indexer);
//		$this->getCustomer()->acceptIndexer($indexer);
//		$this->getAssigned()->acceptIndexer($indexer);
		
		foreach ($this->getNotes() as $note)
			$note->acceptIndexer($indexer);
		foreach ($this->getCategories() as $category)
			$category->acceptIndexer($indexer);
		foreach ($this->getFiles() as $file)
			$indexer->visitFile($file->path());
	}
	
	/**
	 * Returns the user who created this issue
	 * @return	User
	 */
	function getCreator() {
		if (null === ($creator = SyndNodeLib::getInstance($this->data['CREATE_NODE_ID'])))
			$creator = SyndNodeLib::getInstance('user_null.null');
		return $creator;
	}
	
	/**
	 * Sets the creator of this issue
	 * @param	User
	 */
	function setCreator($creator) {
		$this->data['CREATE_NODE_ID'] = $creator->nodeId;
		if (empty($this->data['UPDATE_NODE_ID']))
			$this->data['UPDATE_NODE_ID'] = $creator->nodeId;
	}
	
	/**
	 * Returns the user that reported this issue
	 * @return	User
	 */
	function getCustomer() {
		if (null === ($client = SyndNodeLib::getInstance($this->data['CLIENT_NODE_ID'])))
			$client = SyndNodeLib::getInstance('user_null.null');
		return $client;
	}

	/**
	 * Set the client of this issue
	 * @param	User
	 */
	function setCustomer($customer) {
		$this->data['CLIENT_NODE_ID'] = $customer->nodeId;
	}

	/**
	 * Specifies a hint for this customer
	 * @param	string	Email or phone number to use for hint
	 */
	function setCustomerHint($hint) {
		$this->data['INFO_CUSTOMER_HINT'] = $hint;
	}

	/**
	 * Returns the notes appended to this issue
	 * @return	IteratorAggregate
	 */
	function getNotes() {
		return $this->_notes;
	}
	
	/**
	 * Returns the visible notes
	 * @return	Iterator
	 */
	function getPublishedNotes() {
		$notes = clone $this->getNotes();
		if ($this->_task != null)
			$notes->removeTemporary($this->_storage->getInstance($this->_task));
		if (!$this->isPermitted('admin')) {
			$notes->flush();
			$notes->getDatabaseQuery()->where('flag_protected', 0);
		}
		return $notes;
	}

	/**
	 * Traps child issues and manages the internal buffers
	 */
	function appendChild($child) {
		if (($child instanceof synd_node_task)) {
			$this->_notes->append($child);
			return $child;
		}
		else if (($child instanceof synd_node_issue)) {
			$this->_open = null;
			$this->_issues->append($child);
			return $child;
		}
		else if (($child instanceof synd_type_file)) {
			$this->data['DATA_FILES'][$child->nodeId()] = $child;
			$child->setParent($this);
			return $child;
		}
		return parent::appendChild($child);
	}

	/**
	 * Manages the internal buffers
	 */
	function removeChild($child) {
		if (($child instanceof synd_node_task)) {
			$this->_notes->remove($child);
			$this->_node_onchange_internal();
		}
		else if (($child instanceof synd_node_issue)) {
			$this->_open = null;
			$this->_issues->remove($child);
			$this->_node_onchange_internal();
		}
		return parent::removeChild($child);
	}

	/**
	 * Returns the duration in seconds.
	 * @return	integer
	 */
	function getDuration() {
		return SyndLib::sum(new MethodIterator($this->getNotes()->getIterator(), 'getDuration'));
	}
	
	/**
	 * Returns the estimated duration in seconds.
	 * @return	integer
	 */
	function getEstimate() {
		return $this->data['INFO_ESTIMATE']*60;
	}

	/**
	 * Set the estimated duration
	 * @param	integer	Number of seconds
	 */
	function setEstimate($estimate) {
		return $this->data['INFO_ESTIMATE'] = max(round($estimate/60), 0);
	}

	function toString() {
		if (!empty($this->data['INFO_HEAD']))
			return $this->objectId().': '.tpl_chop($this->data['INFO_HEAD'],65);
		if ($this->isNew()) {
			if (null != ($parent = $this->getParent()))
				return SyndLib::translate('New issue for %s', $parent->toString());
			return SyndLib::translate('New issue');
		}
		return parent::toString();
	}
	
	/**
	 * Returns the title
	 * @return	string
	 */
	function getTitle() {
		if (null != $this->data['INFO_HEAD'])
			return $this->data['INFO_HEAD'];
		return SyndLib::translate('No title');
	}
	
	function setTitle($title) {
		$this->data['INFO_HEAD'] = $title;
	}

	function setInitialSearch($search) {
		$this->data['INFO_INITIAL_QUERY'] = $search;
	}
	
	function isUnmappedCustomer() {
		if (!$this->data['INFO_INITIAL_QUERY'] || ($this->getCustomer() instanceof synd_node_case) || !$this->isPermitted('admin'))
			return false;
		
		$ids = array();
		for ($parent = $this->getParent(); !$parent->isNull() && $parent->isPermitted('write'); $parent = $parent->getParent()) 
			$ids[] = $parent->nodeId;
		
		if (empty($ids))
			return false;
		
		$sql = "
			SELECT 1 FROM synd_project_mapping pm
			WHERE 
				pm.project_node_id IN (".implode(',', $this->_db->quote($ids)).") AND
				pm.query = ".$this->_db->quote($this->data['INFO_INITIAL_QUERY']);
		return !$this->_db->getOne($sql);
	}
	
	function _view_mapCustomer(Request $request, Template $page) {
		if (!$this->isPermitted('admin'))
			throw new ForbiddenException();
		
		if (isset($request['yes'])) {
			$parent = $this->getParent();
			while (!$parent->getParent()->isNull() && $parent->getParent()->isPermitted('write')) 
				$parent = $parent->getParent();
			if (!$parent->isNull())
				$parent->addCustomerMapping($this->getCustomer(), $this->data['INFO_INITIAL_QUERY']);
		}
		else if (isset($request['no'])) {
			$this->data['INFO_INITIAL_QUERY'] = null;
			$this->save();
		}
	}
	
	/**
	 * Returns the issue description
	 * @return	string
	 */
	function getDescription() {
		return $this->getContent()->toString();
	}
	
	/**
	 * Returns an experpt from the description
	 * @return	string
	 */
	function getExcerpt() {
		if (null === $this->_excerpt) {
			$content = $this->getContent();
			$this->_excerpt = '#'.$this->objectId();
			if (null != ($excerpt = tpl_chop($content->toString(), 256))) {
				$this->_excerpt .= ': ';
				$this->_excerpt .= $excerpt;
			}
			$this->_node_onchange_internal();
		}
		return $this->_excerpt;
	}
	
	/**
	 * Sets the issue description
	 * @param	string	Issue description
	 */
	function setDescription($description) {
		$this->getContent()->setContent($description);
	}
	
	/**
	 * Returns the description content object
	 * @return	DomainContent
	 */
	function getContent() {
		if (null == $this->_content) {
			if (null == $this->data['DATA_CONTENT'])
				$this->setContent(SyndType::factory('text'));
			else
				$this->_content = unserialize($this->data['DATA_CONTENT']);
		}
		return $this->_content;
	}
	
	/**
	 * Set the description content object
	 * @param	DomainContent
	 */
	function setContent($content) {
		$content->setParent($this);
		$this->_content = $content;
	}

	/**
	 * Replace issue number in content with link
	 * @param	string	The text to filter
	 * @param	string	The original charset
	 * @return 	string
	 */
	function _callback_filter($text, $charset) {
		$result = tpl_html_format(wordwrap($text, 150));
		$result = SyndLib::runPreloadedHook('issue_filter_text', $result, $this, $text, $charset);
		return preg_replace_callback('/#(\d+)/', array($this, '_callback_filter_replace'), $result);
	}
	
	/**
	 * @access private
	 */
	function _callback_filter_replace($matches){
		if(null !== ($issue = self::findIssue($this->_storage, $matches[1])) && $issue->isPermitted('read'))
			return '<a href="'.tpl_link('issue', $issue->objectId()).'" title="'.tpl_attribute($issue->getTitle()).'">'.$matches[0].'</a>';
		return $matches[0];
	}

	/**
	 * Returns the files attached to this issue
	 * @see		synd_type_file
	 * @return	array	Array of synd_type_file objects
	 */
	function getFiles() {
		if (null != $this->data['DATA_FILES'])
			return $this->data['DATA_FILES'];
		return array();
	}
	
	/**
	 * Returns a file given its id
	 * @see		getFiles();
	 * @return	synd_type_file	Return null if not found
	 */
	function getFile($id) {
		if (isset($this->data['DATA_FILES'][$id]))
			return $this->data['DATA_FILES'][$id];
		return null;
	}
	
	/**
	 * Deletes a file given its id
	 * @see		getFiles();
	 * @param	string	Key of file
	 */
	function unlink($id) {
		if (!isset($this->data['DATA_FILES'][$id]) || !$this->data['DATA_FILES'][$id]->delete()) 
			return false;
		unset($this->data['DATA_FILES'][$id]);
		return true;
	}
	
	function _view_unlink(Request $request, Template $page) {
		if (!$this->isPermitted('write'))
			throw new ForbiddenException();

		if (isset($request['confirm'])) {
			if ($this->unlink($request[2]))
				$this->_save();
			else if (null !== ($task = $this->_storage->getInstance($this->_task))) {
				$task->unlink($request[2]);
				$task->_save();
			}
		}
		else {
			if (null === ($file = $this->getFile($request[2])) && (
				null === ($task = $this->_storage->getInstance($this->_task)) || 
				null === ($file = $task->getFile($request[2]))))
				throw new NotFoundException();
			return array('content' => $page->fetchnode($this,'confirm_delete_file.tpl',array('file' => $file)));
		}
	}

	/**
	 * Attempts to find an issue given an id
	 * @param	DomainStorage	The storage to search
	 * @param	integer			The issue id
	 * @return	synd_node_issue	Returns null if nothing found	
	 */
	static function findIssue($storage, $id) {
		assert('is_numeric($id)');
		if (null !== ($issue = $storage->getInstance('issue.'.$id)))
			return $issue;
			
		$persistent = $storage->getPersistentStorage();
		$database = $persistent->getDatabase();
		
		$sql = "
			SELECT i.node_id FROM synd_issue i
			WHERE i.info_issue_id = ".$database->quote($id);
		if (null !== ($issue = $storage->getInstance($database->getOne($sql))))
			return $issue;
			
		$result = null;
		return $result;
	}
	
	/**
	 * Returns the private key for this issue
	 * @return	string
	 */
	function getPrivateKey() {
		if (null == $this->data['INFO_PRIVATE_KEY']) {
			$this->data['INFO_PRIVATE_KEY'] = md5(uniqid(rand(), true));
			$this->_save();
		}
		return $this->data['INFO_PRIVATE_KEY'];
	}

	/**
	 * Returns an authentication token
	 *
	 * The token is based on the private key and system time() and 
	 * can be used with 
	 *
	 * @see		validateAuthenicationToken()
	 * @return	string
	 */
	function getAuthenticationToken() {
		$ts = round(time()/3600/24);
		return substr(md5($ts.$this->getPrivateKey()),0,16).dechex($ts);
	}
	
	/**
	 * Validates an authentication token
	 * @param	string	Authentication token
	 * @param	integer	Time to live in seconds, default to 30 days
	 * @return	bool
	 */
	function validateAuthenticationToken($token, $ttl = 2592000) {
		$ts = hexdec(substr($token,16));
		return substr(md5($ts.$this->getPrivateKey()),0,16) == substr($token,0,16) && 
			(null == $ttl || $ts*3600*24 >= time()-$ttl && $ts*3600*24 <= time());
	}

	/**
	 * Return issue priority
	 * @return	integer	Priority level {0,1,2}
	 */
	function getPriority() {
		return $this->data['INFO_PRIO'];
	}

	/**
	 * Set issue priority
	 * @param	integer	Priority level {0,1,2}
	 */
	function setPriority($priority) {
		$this->data['INFO_PRIO'] = $priority;
	}
	
	/**
	 * Returns the valid priority options based on the current security context
	 * @return	array
	 */
	function getDefinedPriorities() {
		if ($this->isPermitted('admin'))
			return Module::getInstance('issue')->getDefinedPriorities();
		return array();
	}
	
	/**
	 * Collects feedback from users when issue have been closed
	 * @param	string	Authentication token
	 * @param	integer	Rating of solution {0,1}
	 */
	function _view_feedback(Request $request, Template $page) {
		global $synd_user;
		if (!isset($request[2], $request[3]) || (0 != $request[3] && 1 != $request[3]))
			throw new InvalidArgumentException();
		if (!$this->validateAuthenticationToken($request[2]))
			throw new ForbiddenException();
		
		$sql = "
			SELECT 1 FROM synd_issue_feedback f
			WHERE 
				f.issue_node_id = ".$this->_db->quote($this->nodeId)." AND 
				f.authentication_token = ".$this->_db->quote($request[2]);
		if (null != $this->_db->getOne($sql)) 
			return array('content' => $page->fetchnode($this,'confirm_feedback_invalid.tpl'));
		
		$sql = "
			INSERT INTO synd_issue_feedback
			(issue_node_id, authentication_token, info_rating, ts_create)
			VALUES
			(".$this->_db->quote($this->nodeId).",
			 ".$this->_db->quote($request[2]).",
			 ".($request[3] ? 1 : 0).",
			 ".time().")";
		$this->_db->query($sql);
		
		return array('content' => $page->fetchnode($this,'confirm_feedback_received.tpl',array('feedback' => $request[3])));
	}
	
	/**
	 * The unsatisfied user might choose to reopen the issue and provide optional feedback for doing so
	 * @param	string	Authentication token
	 */
	function _view_reopen(Request $request, Template $page) {
		global $synd_user;
		if (!isset($request[2]))
			throw new InvalidArgumentException();
		if (!$this->validateAuthenticationToken($request[2]))
			throw new ForbiddenException();
		
		$this->setStatus(self::ACTIVE);
		$this->_save();
		
		if (!empty($request['note'])) {
			$note = $this->appendChild($this->_storage->factory('task'));
			$note->setDescription($request['note']);
			$note->save();
		}
		
		return array('content' => $page->fetchnode($this,'confirm_feedback_reopen.tpl',array('note'=>$request['note'])));
	}

	/**
	 * Returns the MailNotifier helper for this node
	 * @access	protected
	 * @return	synd_type_mail_notifier
	 */
	function getMailNotifier() {
		if (null === $this->_mailNotifier) {
			if (null == $this->data['DATA_NOTIFIER'] || false === ($this->_mailNotifier = unserialize($this->data['DATA_NOTIFIER']))) {
				$this->_mailNotifier = SyndType::factory('mail_notifier');
				$this->_mailNotifier->setParent($this);
			}
			$this->_node_onchange_internal();
		}
		return $this->_mailNotifier;
	}
	
	/**
	 * Removes a mail listener
	 * @param	string	Name of event
	 * @param	string	Instance id of listener
	 */
	function _view_delListener(Request $request, Template $page) {
		$project = $this->getParent();
		if (!$project->isPermitted('write'))
			throw new ForbiddenException();
		if (!isset($request[2], $request[3]))
			throw new InvalidArgumentException();
		if (null !== ($listener = SyndLib::getInstance($request[3]))) {
			$mailNotifier = $this->getMailNotifier();
			$mailNotifier->removeListener($request[2], $listener);
			$this->_save();
		}
	}

	/**
	 * Returns the EventLogger helper for this node
	 * @access	protected
	 * @return	synd_type_issue_event_logger
	 */
	function getEventLogger() {
		if (null === $this->_eventLogger && (null == $this->data['DATA_EVENTLOG'] ||
			false === ($this->_eventLogger = unserialize($this->data['DATA_EVENTLOG'])))) {
			$this->_eventLogger = SyndType::factory('issue_event_logger');
			$this->_eventLogger->setParent($this);
		}
		return $this->_eventLogger;
	}
	
	/**
	 * Fires an event on this node.
	 * @param	Event to fire
	 */
	function fireEvent($event) {
		$mailNotifier = $this->getMailNotifier();
		$mailNotifier->handleEvent($event);
		
		$eventLogger = $this->getEventLogger();
		$eventLogger->handleEvent($event);
		
		$this->bubbleEvent($event);
		$this->_node_onchange_internal();
	}
	
	/**
	 * Cancel the issue and append a task saying so
	 */
	function cancel() {
		$task = $this->appendChild(SyndNodeLib::factory('task'));
		$content = $task->getContent();
		$content->setContent(SyndLib::translate('Issue has been cancelled'));
		$task->save();

		$this->setStatus(self::CLOSED);
		$this->fireEvent(SyndType::factory('oncancel', $this));
	}
	
	function _view_cancel(Request $request, Template $page) {
		if (!$this->isPermitted('write'))
			throw new ForbiddenException();
		if (!isset($request['confirm']))
			return array('content' => $page->fetchnode($this, 'confirm_cancel.tpl'));
		$this->cancel();
		$this->_save();
	}
	
	/**
	 * Includes the description as plaintext and any temporary task
	 */
	function getCompositeData() {
		$data = parent::getCompositeData();
		$content = $this->getContent();
		$data['content'] = $content->toString();

		// Merge data from temporary task
		if (null !== ($task = $this->_storage->getInstance($this->_task)))
			$data['task'] = $task->getCompositeData();

		// Default to waiting if non-active
		if ($data['INFO_STATUS'] < self::ACTIVE)
			$data['INFO_STATUS'] = self::PENDING;
		
		return $data;
	}

	function validate($data) {
		global $synd_user;
		require_once 'core/lib/SyndHTML.class.inc';
		require_once 'core/lib/SyndDate.class.inc';
		$errors = array();

		if (isset($data['TS_RESOLVE_BY'])) {
			if (!SyndDate::checkdate($data['TS_RESOLVE_BY']))
				$errors['TS_RESOLVE_BY'] = SyndLib::translate('Invalid date');
			else if ($this->isNew() && strtotime($data['TS_RESOLVE_BY']) < strtotime('+1 days 00:00'))
				$errors['TS_RESOLVE_BY'] = SyndLib::translate('Please specify a date further ahead in time');
			unset($data['TS_RESOLVE_BY']);
		}
		
		// Validate new note
		if (!empty($data['task']) && count(array_filter($data['task'],'strlen'))) {
			if (null === ($task = $this->_storage->getInstance($this->_task))) {
				$task = $this->appendChild(SyndNodeLib::factory('task'));
				$this->_task = $task->nodeId;
			}
			$errors = array_merge($errors, $task->validate($data['task']));
		}

		// Assign and remove categories
		if (isset($data['categories'])) {
			$selected = isset($data['categories']['selected']) ? (array)$data['categories']['selected'] : array();
			$mandatory = SyndLib::filter($this->getParent()->getCategoriesRecursive(), 'isMandatory');
			
			foreach (SyndLib::sort($mandatory) as $category) {
				$children = SyndLib::collect($category->getCategoriesRecursive(), 'nodeId');
				$children[] = $category->nodeId;
				
				if (!count(array_intersect($children, $selected)))
					$errors["categories[{$category->nodeId}]"] = SyndLib::translate('Category %s must be selected', $category->toString());
			}
		}

		if (isset($data['PARENT_NODE_ID'])) {
			$parent = $this->getParent();
			$newParent = $this->_storage->getInstance($data['PARENT_NODE_ID']);
			if (null == $newParent || !$newParent->isPermitted('read') && $newParent->nodeId != $parent->nodeId)
				$errors['PARENT_NODE_ID'] = SyndLib::translate('Invalid project selected');
		}

		if (isset($data['ASSIGNED_NODE_ID']) && !$this->isPermitted('assign'))
			$errors['ASSIGNED_NODE_ID'] = SyndLib::translate('Not permitted to assign this issue');
		
		if (isset($data['INFO_ESTIMATE']))
			$data['INFO_ESTIMATE'] = SyndDate::evalDuration($data['INFO_ESTIMATE']);
		
		
		// Search for client
		if ($this->isPermitted('issue_client') && isset($data['client'], $data['prevClient'])) {
			$module = Module::getInstance('user');
			$client = $this->getCustomer();

			if ('' == $data['client']) {
				$data['CLIENT_NODE_ID'] = $synd_user->nodeId;
				unset($data['client']);
			}
			else if (strlen($data['client']) > 2 && '"' == $data['client']{0} && '"' == $data['client']{strlen($data['client'])-1}) {
				$data['CLIENT_NODE_ID'] = 'user_case.'.trim($data['client'],'"');
				unset($data['client']);
			}	
			else if ($client->isNull() || $data['client'] != $client->getLogin() && $data['client'] != $client->toString()) {
				if (null !== ($newClient = $module->resolveLogin($data['client'])) ||
					null !== ($newClient = $module->resolveEmail($data['client'])))
					$data['CLIENT_NODE_ID'] = $newClient->nodeId;
				else if ($data['prevClient'] != $data['client'] || !isset($data['CLIENT_NODE_ID'])) {
					$collection = $module->findUsers($data['client']);
					$errors['client'] = SyndLib::translate('Please select a customer');
					$errors['client_matches'] = $collection->getContents(0,5);
				}
				else
					unset($data['client']);
			}
		}

		unset($data['INFO_STATUS']);

		SyndHTML::validateFile($data, $errors, 'file');
		return array_merge(parent::validate($data), $errors,
			(array)SyndLib::runHook('issue_data_validate', $this, $data));
	}
	
	function addTask($text, $duration) {
		$task = $this->appendChild(SyndNodeLib::factory('task'));
		$task->setDescription($text);
		$task->setDuration($duration);
		$this->_task = $task->nodeId;
	}
	
	function merge($data) {
		global $synd_user;
		require_once 'core/lib/SyndDate.class.inc';
		
		// Merge flag to notify user on events
		if (isset($data['INFO_ESTIMATE'])) {
			$mailNotifier = $this->getMailNotifier();

			// Remove user from events
			$removeListeners = empty($data['notify']) ? 
				$mailNotifier->getListenerEvents($synd_user) :
				array_diff($mailNotifier->getListenerEvents($synd_user), array_keys($data['notify']));
			foreach ($removeListeners as $event)
				$mailNotifier->removeListener($event, $synd_user);

			// Add user to events
			if (!empty($data['notify'])) {
				foreach (array_diff(array_keys($data['notify']), $mailNotifier->getListenerEvents($synd_user)) as $event)
					$mailNotifier->addListener($event, $synd_user);
			}
		}

		// Set issue status
		if ($this->isPermitted('status') && isset($data['INFO_STATUS'])) {
			$this->setStatus($data['INFO_STATUS']);
			unset($data['INFO_STATUS']);
		}

		// Set parent project
		if (isset($data['PARENT_NODE_ID'])) {
			$this->setParent($this->_storage->getInstance($data['PARENT_NODE_ID']));
			unset($data['PARENT_NODE_ID']);
		}
		
		// Set assigned user
		if (isset($data['ASSIGNED_NODE_ID'])) {
			if (null !== ($assigned = SyndNodeLib::getInstance($data['ASSIGNED_NODE_ID'])))
				$this->setAssigned($assigned);
			else
				$this->setAssigned(SyndNodeLib::getInstance('user_null.null'));
			unset($data['ASSIGNED_NODE_ID']);
		}

		// Search for and assign client
		if ($this->isPermitted('issue_client') && isset($data['client'], $data['prevClient'])) {
			$module = Module::getInstance('user');
			$client = $this->getCustomer();

			if ('' == $data['client']) {
				$data['CLIENT_NODE_ID'] = $synd_user->nodeId;
				unset($data['client']);
			}
			else if (strlen($data['client']) > 2 && '"' == $data['client']{0} && '"' == $data['client']{strlen($data['client'])-1}) {
				$data['CLIENT_NODE_ID'] = 'user_case.'.trim($data['client'],'"');
				unset($data['client']);
			}			
			else if ($client->isNull() || $data['client'] != $client->getLogin() && $data['client'] != $client->toString()) {
				if (null !== ($newClient = $module->resolveLogin($data['client'])) ||
					null !== ($newClient = $module->resolveEmail($data['client'])))
					$data['CLIENT_NODE_ID'] = $newClient->nodeId;
				else
					unset($data['client']);
			}
		}

		// Default client to logged in user
		if (empty($data['CLIENT_NODE_ID']) && empty($this->data['CLIENT_NODE_ID']))
			$data['CLIENT_NODE_ID'] = $synd_user->nodeId;
		
		// Evaluate estimate
		if (isset($data['INFO_ESTIMATE']))
			$data['INFO_ESTIMATE'] = SyndDate::evalDuration($data['INFO_ESTIMATE']);
			
		// Attach new file
		if (!empty($data['file']['tmp_name']))
			$file = SyndType::factory('file', $data['file']['tmp_name'], $data['file']['name']);
	
		// Append new note
		if (!empty($file) || !empty($data['task']) && count(array_filter($data['task'],'strlen'))) {
			if (null === ($task = $this->_storage->getInstance($this->_task))) {
				$task = $this->appendChild(SyndNodeLib::factory('task'));
				$this->_task = $task->nodeId;
			}
			if (!empty($file))
				$task->appendChild($file);
			if (!empty($data['task']))
				$task->merge($data['task']);
		}
		
		// Assign and remove categories
		if (isset($data['categories'])) {
			$previous = SyndLib::collect($this->getCategories(), 'nodeId');
			$selected = isset($data['categories']['selected']) ? (array)$data['categories']['selected'] : array();
			foreach (array_diff($previous, $selected) as $id)
				$this->removeCategory($this->_storage->getInstance($id));
			foreach (array_diff($selected, $previous) as $id)
				$this->addCategory($this->_storage->getInstance($id));
		}
		
		// Description content
		if (isset($data['content'])) {
			$content = $this->getContent();
			$content->setContent($data['content']);
		}

		if (isset($data['TS_RESOLVE_BY']))
			$data['TS_RESOLVE_BY'] = strtotime($data['TS_RESOLVE_BY']);

		// Merge data info $this->data
		parent::merge($data);
		SyndLib::runHook('issue_data_merge', $this, $data);

		// Send notifies to specific addresses
		if (isset($data['mail'])) {
			$mailNotifier = $this->getMailNotifier();

			$issues = empty($_REQUEST['selection']) ? 
				array($this) : SyndLib::getInstances($_REQUEST['selection']);
			
			if (isset($data['mail']['client'])) {
				foreach (array_keys($issues) as $key) {
					if (($issues[$key] instanceof synd_node_issue)) {
						$mailNotifier->addTemporary($issues[$key]->getCustomer());
					}
				}
			}
	
			if (isset($data['mail']['assigned'])) {
				foreach (array_keys($issues) as $key) {
					if (($issues[$key] instanceof synd_node_issue)) {
						$mailNotifier->addTemporary($issues[$key]->getAssigned());
					}
				}
			}
				
			if (!empty($data['mail']['address'])) {
				$module = Module::getInstance('user');
				foreach (preg_split('/[^\w\-+.@]+/', $data['mail']['address']) as $email) {
					if (false === strpos($email, '@') && null !== ($user = $module->resolveLogin($email))) {
						$mailNotifier->addTemporary($user);
					}
					else {
						$mailNotifier->addTemporaryEmail($email);
					}
				}
			}

			// Attach files to outgoing emails
			if (isset($data['attachment'])) {
				foreach ((array)$data['attachment'] as $id) {
					if (null !== ($file = SyndLib::getInstance($id)) && null !== ($parent = $file->getParent()) && $parent->isPermitted('read'))
						$mailNotifier->addAttachment($file);
				}
				unset($data['attachment']);
			}
			
			unset($this->data['mail']);
		}
	}
	
	/**
	 * Returns the direct email address to this issue
	 * @return	string	Returns null
	 */
	function getDirectAddress() {
		$parent = $this->getParent();
		$project = $parent->getAbsoluteEmail();
		if (!$project->isNull())
			return str_replace('@', '+'.$this->objectId().'@', $project->getEmail());
		return null;
	}
	
	/**
	 * Returns current user or project email depending on settings
	 * @param	synd_type_event	Event to send for
	 * @return	array
	 */
	function getEmailSenders() {
		global $synd_user;
		$parent = $this->getParent();
		$project = $parent->getAbsoluteEmail();
		$senders = array();
		
		if ($parent->isDisplaySender() && null != $synd_user->getEmail()) {
			$senders[] = $this->_formatEmailSender($synd_user);
			if (!$project->isNull()) {
				$senders[] = $project->isIssueSender() ? 
					str_replace('@', '+'.$this->objectId().'@', $this->_formatEmailSender($project)) :
					$this->_formatEmailSender($project);
			}
		}
		else if (!$project->isNull()) {
			$senders[] = $project->isIssueSender() ? 
				str_replace('@', '+'.$this->objectId().'@', $this->_formatEmailSender($project)) :
				$this->_formatEmailSender($project);
		}
		else if (null != $synd_user->getEmail())
			$senders[] = $this->_formatEmailSender($synd_user);
		else {
			if (!isset($_SERVER['SERVER_NAME']))
				$domain = 'example.com';
			else {
				$domain = false !== ($i = strpos($_SERVER['SERVER_NAME'],'.')) ? 
					substr($_SERVER['SERVER_NAME'],$i+1) : $_SERVER['SERVER_NAME'];
			}
			$senders[] = "noreply@$domain";
		}
		
		return $senders;
	}
	
	/**
	 * Returns the direct email address to this issue
	 * @return	string	Returns null
	 */
	function getEnvelopeAddress() {
		return $this->getDirectAddress();
	}
	
	/**
	 * @access	private
	 */
	function _formatEmailSender($user) {
		return $user->isNull() ? null : '"'.str_replace('"','',$user->toString()).'" <'.$user->getEmail().'>';
	}
	
	function getEmailSubject() {
		return SyndLib::translate('Re: %s %s', $this->getTitle(), $this->getEmailSubjectId());
	}
	
	function getEmailSubjectId() {
		if (null !== ($module = Module::getInstance('issue')) && null != ($namespace = $module->getNamespace()))
			return '#'.$this->objectId().'@'.$namespace;
		return '#'.$this->objectId();
	}

	/**
	 * Renders the email body
	 * @param	synd_type_event	Event to be sent
	 * @param	array			Users to receive email
	 * @return	string
	 */
	function getEmailBody($event, $users) {
		return $this->getRenderedTemplate($event->classId(), array($this, '_callback_render_email'));
	}

	/**
	 * Renders a template using the specified callback for variable substitutions
	 * @param	string	Template name to render
	 * @param	mixed	Variable callback
	 * @return	string
	 */
	function getRenderedTemplate($name, $callback) {
		$parent = $this->getParent();
		$template = new IssueTemplate($callback);
		if (null == ($locale = SyndLib::runHook('getlocale')))
			$locale = 'en';
		global $synd_config;
		$page = new Template(array_reverse($synd_config['dirs']['design']));
		return $template->render($parent->getTemplate($page, $name, $locale));
	}

	/**
	 * Variable substitution callback
	 * @see		synd_node_project::getRenderedTemplate()
	 * @param	string	The variable to fetch
	 * @return	string
	 */
	function _callback_render_email($variable) {
		switch (strtoupper($variable)) {
			case 'ID':
				return $this->objectId();
			case 'TITLE':
				return $this->getTitle();
			case 'DESCRIPTION':
				return $this->getDescription();
			case 'PROJECT':
				$project = $this->getParent();
				return $project->toString();
			case 'ASSIGNED':
				$assigned = $this->getAssigned();
				return $assigned->toString();
			case 'ASSIGNED_CONTACT_INFO':
				$assigned = $this->getAssigned();
				return $assigned->getContact();
			case 'CUSTOMER':
				$customer = $this->getCustomer();
				return $customer->toString();
			case 'CUSTOMER_CONTACT_INFO':
				$customer = $this->getCustomer();
				return $customer->getContact();
			case 'NOTE':
				if (null != ($note = SyndLib::max(new MethodObjectIterator($this->getNotes()->getIterator(), 'getUpdateTime'))))
					return $note->getDescription();
				break;
			case 'LINK':
				return tpl_request_host().tpl_view($this->getHandler(),$this->objectId(),$this->getAuthenticationToken());
			case 'FEEDBACK_LINK_YES':
				return tpl_request_host().tpl_view($this->getHandler(),'invoke',$this->nodeId,'feedback',$this->getAuthenticationToken(),1);
			case 'FEEDBACK_LINK_NO':
				return tpl_request_host().tpl_view($this->getHandler(),'invoke',$this->nodeId,'feedback',$this->getAuthenticationToken(),0);
		}
	}

	/**
	 * Appends email notifaction info to redirect uri
	 */
	function _view_edit(Request $request, Template $page) {
		$mres = parent::_view_edit($request, $page);
		if (!empty($request['post']) && isset($mres['redirect']) && 0 === strpos($mres['redirect'], tpl_view($this->getHandler()))) {
			$mres['redirect'] = rtrim(preg_replace('/\bmailed(\[|%5B)[^&]*/i', '', $mres['redirect']), '?&');
			if (null !== $this->_mailNotifier && count($listeners = $this->_mailNotifier->getActiveListeners()))
				$mres['redirect'] .= (false === strpos($mres['redirect'],'?') ? '?' : '&').http_build_query(array('mailed'=>$listeners));
		}
		return $mres;
	}
	
	/**
	 * Creates a new child issue and redirect to its edit view
	 */
	function _view_newIssue(Request $request, Template $page) {
		global $synd_user;
		if (!$this->isPermitted('write'))
			throw new ForbiddenException();
		$project = $this->getParent();
		$issue = $this->appendChild($project->appendChild($this->_storage->factory('issue')));
		$issue->setCustomer($this->getCustomer());
		return array('redirect' => tpl_view($issue->getHandler(),'edit',$issue->nodeId));
	}
	
	/**
	 * Clones this issue and redirects to its edit view
	 */
	function _view_clone(Request $request, Template $page) {
		if (!$this->isPermitted('write'))
			throw new ForbiddenException();
		$issue = $this->copy();
		return array('redirect' => tpl_view($issue->getHandler(),'edit',$issue->nodeId));
	}
	
	/**
	 * Autosuggest clients callback
	 * @see		synd_module_user::findSuggestedUsers()
	 */
	function _public_findSuggestedUsers($query) {
		if (!$this->isPermitted('write'))
			throw new ForbiddenException();
		ignore_user_abort(false);
		return Module::getInstance('issue')->findSuggestedUsers($query);
	}

	/**
	 * Wordwraps a text but keeps leading "> > >" quoting intact (for email purposes)
	 * @param	string	String to wrap
	 * @param	int		Witdh to wrap at
	 * @param	string	String to insert at cut
	 * @return	string
	 */
	static function wordwrap($str, $width = 75, $break = "\r\n") {
		return preg_replace_callback('/([ \t>]*)(.{'.$width.',})/', array(new IssueWordwrapDelegate($width, $break), 'replace'), $str);
	}
	
	/**
	 * Dispatches the reply/forwarded email
	 */
	function _view_send(Request $request, Template $page) {
		if (!$this->isPermitted('write'))
			throw new ForbiddenException();
		if (null == ($to = $this->_findEmailAddresses($request['to'])))
			return;
		$receivers = array($to);
		
		if (empty($request['redirect'])) {
			$mime = Mime::createTextpart($request['message']);

			// Attach specified files
			if (!empty($request['attachment'])) {
				$mime = Mime::createMultipart(array($mime));
				foreach ((array)$request['attachment'] as $id) {
					if (null !== ($file = SyndLib::getInstance($id)) && null !== ($parent = $file->getParent()) && $parent->isPermitted('read'))
						$mime->addPart(Mime::createTextpart(file_get_contents($file->path()), $file->toString()));
				}
			}

			if (count($senders = $this->getEmailSenders()) > 1) 
				$mime->setHeader('Sender', reset($senders));
			$mime->setHeader('From', implode(', ', $senders));
			$mime->setHeader('Subject', $request['subject']);

			if (null != ($cc = $this->_findEmailAddresses($request['cc']))) {
				$mime->setHeader('Cc', $cc);
				$receivers[] = $cc;
			}
			if (null != ($bcc = $this->_findEmailAddresses($request['bcc']))) {
				$mime->setHeader('Bcc', $bcc);
				$receivers[] = $bcc;
			}

			// Send email
			$headers = array();
			foreach ($mime->getMessageHeaders() as $name => $value)
				$headers[] = $name.': '.MimeHeader::encodeHeader($name, $value);

			$flags = null != ($direct = $this->getDirectAddress()) ? "-f $direct" : null;
			mail($to, $request['subject'], $mime->getMessageBody(), implode("\r\n", $headers), $flags);

			// Append email to issue
			$note = $this->appendChild($this->_storage->factory('task'));
			$note->setDuration(isset($request['duration']) ? 60*(int)$request['duration'] : 60*20);

			$mime->setHeader('To', $to);
			$note->setContent(SyndType::factory('email', $mime));
			$note->save();
		}
		else {
			if (null === ($email = SyndLib::getInstance($request['redirect'])))
				throw new NotFoundException();
			if (null === ($parent = $email->getParent()) || !$parent->isPermitted('read'))
				throw new ForbiddenException();
			
			$mime = clone $email->getMessage();
			$mime->setHeader('Resent-Date', ucwords(tpl_strftime('%a, %d %b %Y %H:%M:%S %O (%Z)')));
			if (count($senders = $this->getEmailSenders()) > 1) 
				$mime->setHeader('Resent-Sender', reset($senders));
			$mime->setHeader('Resent-From', implode(', ', $senders));
			$mime->setHeader('Resent-To', $to);
			
			$headers = array();
			foreach ($mime->getMessageHeaders() as $name => $value) {
				if ('to' != strtolower($name))
					$headers[] = $name.': '.MimeHeader::encodeHeader($name, $value);
			}
				
			$flags = null != ($direct = $this->getDirectAddress()) ? "-f $direct" : null;
			mail($mime->getHeader('To'), $mime->getHeader('Subject'), $mime->getMessageBody(), implode("\r\n", $headers), $flags);
		}

		// Set issue status
		if (isset($request['data']['INFO_STATUS']) && $request['data']['INFO_STATUS'] != $this->data['INFO_STATUS']) {
			$this->setStatus($request['data']['INFO_STATUS']);
			$this->_save();
		}
		
		// Ensure that the green "Sent email to" box is displayed
		$redirect = rtrim(preg_replace('/\bmailed(\[|%5B)[^&]*/i', '', tpl_uri_return()), '?&');
		$redirect .= (false === strpos($redirect,'?') ? '?' : '&').http_build_query(array('mailed'=>$receivers));
		return array('redirect' => $redirect);
	}
	
	function _view_forward(Request $request, Template $page) {
		if (!$this->isPermitted('write'))
			throw new ForbiddenException();

		return array('content' => $page->fetchnode($this, 'mail/forward.tpl', array(
			'content'	=> $this->getContent(),
			'files'		=> $this->getFiles())));
	}

	function _view_reply(Request $request, Template $page) {
		if (!$this->isPermitted('write'))
			throw new ForbiddenException();

		return array('content' => $page->fetchnode($this, 'mail/reply.tpl', array(
			'to'		=> $this->getCreator()->getEmail(),
			'content'	=> $this->getContent(), 
			'files'		=> $this->getFiles(),
			)));
	}
	
	/**
	 * @access	private
	 */
	function _findEmailAddresses($string) {
		$module = Module::getInstance('user');
		$result = null;
		
		foreach (preg_split('/\s*[,;]\s*/', $string) as $email) {
			if (null != $result)
				$result .= ', ';
			if (false === strpos($email, '@') && (
				null !== ($user = $module->resolveEmail($email)) || 
				null !== ($user = $module->resolveLogin($email))))
				$result .= '"'.$user->toString().'" <'.$user->getEmail().'>';
			else
				$result .= $email;
		}
		
		return $result;
	}

	/**
	 * Bubble onchange to parent issue
	 */
	function _node_onchange() {
		$this->_excerpt = null;
		parent::_node_onchange();
		$this->getParentIssue()->_node_onchange_child();
	}

	function _node_onchange_child() {
		$this->_open = null;
		$this->_notes->flush();
		$this->_issues->flush();
		parent::_node_onchange_child();
	}

	/**
	 * Loads the content, logger and file instances
	 */
	function _hook_loadInstance($id) {
		$content = $this->getContent();
		if ($content->getInstanceId() == $id)
			return $content;

		$logger = $this->getEventLogger();
		if (null !== ($instance = $logger->_hook_loadInstance($id)))
			return $instance;
	
		foreach (array_keys($files = $this->getFiles()) as $key) {
			if ($files[$key]->getInstanceId() == $id)
				return $files[$key];
		}
	
		$result = null;
		return $result;
	}
	
	function _entity_onpreflush() {
		global $synd_user;
		parent::_entity_onpreflush();
		SyndLib::runHook('issue_onpreflush', $this);
		
		if (!$synd_user->isNull())
			$this->data['UPDATE_NODE_ID'] = $synd_user->nodeId;
		if (!isset($this->data['TS_RESOLVE_BY']))
			$this->data['TS_RESOLVE_BY'] = $this->getParent()->getDefaultResolveBy();
		$this->data['TS_UPDATE'] = time();

		if (null != $this->data['ISSUE_NODE_ID'] && null == $this->_storage->getInstance($this->data['ISSUE_NODE_ID'])) {
			$this->data['ISSUE_NODE_ID'] = null;
		}
		
		// Store attribute objects that have been used
		if (null !== $this->_mailNotifier) {
			$this->_mailNotifier->send();
			$this->data['DATA_NOTIFIER'] = serialize($this->_mailNotifier);
			unset($this->_mailNotifier);
			$this->_mailNotifier = null;
		}

		if (null !== $this->_eventLogger) 
			$this->data['DATA_EVENTLOG'] = serialize($this->_eventLogger);
		if (null !== $this->_content) 
			$this->data['DATA_CONTENT'] = serialize($this->_content);
		if (null !== $this->_attributes) 
			$this->data['DATA_ATTRIBUTES'] = empty($this->_attributes) ? null : serialize($this->_attributes);

		foreach ($this->getNotes() as $note) {
			if ($note->isNew())
				$note->save();
		}

		foreach ($this->getChildren() as $issue) {
			if ($issue->isNew())
				$issue->save();
		}
	}

	/**
	 * Syncs the keywords set
	 */
	function _entity_onpostflush() {
		global $synd_user;
		parent::_entity_onpostflush();
		
		$this->_notes->flush();
		$this->_issues->flush();
		$this->_categories->flush();
	}

	/**
	 * Deletes files from disk
	 */
	function _node_onpredelete() {
		SyndLib::invoke($this->getFiles(),'delete');
		parent::_node_onpredelete();
	}
}


/**
 * @access	private
 */
class IssueTemplate {
	var $_callback = null;
	
	function __construct($callback) {
		$this->_callback = $callback;
	}
	
	function render($template) {
		return preg_replace_callback('/((?<=^|\n)[^\r\n]*?)?\{\s*\$\s*(\w+)\s*\}/', array($this, '_callback_variable'), $template);
	}
	
	function _callback_variable($matches) {
		$value = call_user_func($this->_callback, $matches[2]);
		if (false !== strpos($value, "\n"))
			return preg_replace('/(^|\n)/', '\0'.addslashes($matches[1]), $value);
		return $matches[1].$value;
	}
}

/**
 * @access	private
 */
class IssueWordwrapDelegate {
	private $_width = null;
	private $_break = null;
	
	function __construct($width, $break) {
		$this->_width = $width;
		$this->_break = $break;
	}
	
	function replace($m) {
		$str = wordwrap($m[2], $this->_width, $this->_break);
		if ('' != $m[1])
			$str = $m[1].str_replace("\n", "\n".$m[1], $str);
		return $str;
	}
}

class synd_event_view extends AbstractDomainEvent {
	function __toString() {
		return 'View issue';
	}

	function getDescription() {
		return 'Every time an issue is displayed';
	}
}

class synd_event_mail extends AbstractDomainEvent {
	function __toString() {
		return 'E-mail';
	}

	function getDescription() {
		return 'When an issue is created from an incoming E-mail message';
	}
}

class synd_event_mail_note extends AbstractDomainEvent {
	function __toString() {
		return 'E-mail comment';
	}

	function getDescription() {
		return 'When a new comment is created from an incoming E-mail message';
	}
}
