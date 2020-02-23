<?php
require_once 'core/model/DatabaseEntity.class.inc';
require_once 'core/model/Collection.class.inc';
require_once 'core/model/node/issue.class.inc';

class synd_node_project extends AbstractDatabaseEntity implements Collection {
	/**
	 * @access	private
	 */
	var $_mailNotifier = null;

	/**
	 * Cached issue collection
	 * @access	private
	 */
	var $_collection = null;

	/**
	 * Child projects
	 * @var		AggregateEntityCollection
	 */
	protected $_projects = null;
	
	/**
	 * Project members
	 * @var		AssociateEntityCollection
	 */
	protected $_members = null;
	
	/**
	 * Cached keywords
	 * @access	private
	 */
	var $_keywords = null;
	
	/**
	 * @access	private
	 */
	var $_attributes = null;

	/**
	 * Buffered template contents
	 * @access	private
	 */
	var $_templates = null;
	
	/**
	 * Shutdown hook attached
	 * @access	private
	 */
	var $_shutdown = false;
	
	function __construct($storage, $database, $data) {
		parent::__construct($storage, $database, $data);
		$this->_projects = $this->aggregate(__CLASS__);
		$this->_members = $this->associate('synd_project_member');
	}

	function __factory() {
		global $synd_user;
		$this->data['NODE_ID'] = $this->classId().'.'.$this->_db->nextId('synd_project');
		$this->data['TS_CREATE'] = $this->data['TS_UPDATE'] = time();
		$this->data['CREATE_NODE_ID'] = $this->data['UPDATE_NODE_ID'] = $synd_user->nodeId;
		parent::__factory();
	}

	function __clone() {
		global $synd_user;
		$members = $this->_members->getIterator();

		$this->data['NODE_ID'] = $this->classId().'.'.$this->_db->nextId('synd_project');
		$this->data['TS_CREATE'] = $this->data['TS_UPDATE'] = time();
		$this->data['CREATE_NODE_ID'] = $this->data['UPDATE_NODE_ID'] = $synd_user->nodeId;
		
		parent::__clone();

		$this->_projects = $this->aggregate(__CLASS__);
		$this->_members = $this->associate('synd_project_member');

		foreach ($members as $member)
			$this->_members->append($member);
	}
	
	function __sleep() {
		$vars = parent::__sleep();
		unset($vars[array_search('_shutdown', $vars)]);
		unset($vars[array_search('_collection', $vars)]);
		return $vars;
	}

	static function getEntityQuery(DomainStorage $storage) {
		return self::createQuery($storage, 'synd_project');
	}

	function nodeId() {
		return $this->data['NODE_ID'];
	}

	function hasPermission($user, $perm, $arg = null) {
		if ('read' == $perm && null != $arg)
			return false;
		if ('append' == $perm) {
			if ((($arg instanceof synd_node_project) || ($arg instanceof synd_node_period)) && 
				$this->hasPermission($user, 'write'))
				return true;
			if (($arg instanceof synd_node_issue) && $this->hasPermission($user, 'read'))
				return true;
			if (($arg instanceof synd_node_keyword) && $this->hasPermission($user, 'admin'))
				return true;
		}
			
		if ('remove' == $perm && $this->hasPermission($user, 'read'))
			return true;
		if ('write' == $perm && $this->isNew())
			return true;

		if (parent::hasPermission($user, $perm, $arg))
			return true;
			
		if ('merge' == $perm && in_array($arg, array(
			'PARENT_NODE_ID','INFO_PROJECT_ID','INFO_HEAD','INFO_DESC','INFO_EMAIL',
			'INFO_DEFAULT_CLIENT','INFO_DEFAULT_RESOLVE','INFO_DEFAULT_REOPEN','INFO_CLEANUP_CUTOFF',
			'INFO_COST_CENTER','INFO_PROJECT_NUMBER',
			'FLAG_INHERIT_MEMBERS','FLAG_HIDE_ISSUES','FLAG_DISCARD_SPAM',
			'FLAG_DISPLAY_SENDER','FLAG_ISSUE_SENDER','FLAG_RECEIPT',
			'FLAG_ARCHIVE','FLAG_INHERIT_CATEGORIES')))
			return $this->hasPermission($user, 'write');

		// Inject 'monitor' permission between 'read' and 'write'
		if ('read' == $perm && $this->hasPermission($user, 'monitor', $arg) ||
			'monitor' == $perm && $this->hasPermission($user, 'write', $arg))
			return true;

		// Inject 'manage' permission between 'write' and 'admin'
		if ('write' == $perm && $this->hasPermission($user, 'manage', $arg) ||
			'manage' == $perm && $this->hasPermission($user, 'admin', $arg))
			return true;
		
		return false;
	}

	function hasAuthorization(Instance $user, Instance $subject) {
		return $this->isMember($user) || parent::hasAuthorization($user, $subject);
	}

	/**
	 * Adds the 'manage' permission
	 */
	function getDefinedPermissions() {
		return array(
			'read'		=> SyndLib::translate('Allows customers to view their own issues and create new ones'),
			'monitor'	=> SyndLib::translate('Read access to this project and all contained issues'),
			'write'		=> SyndLib::translate('Write access to this project and all contained issues'),
			'manage'	=> SyndLib::translate('Write access as well as viewing unassigned issues from the "Unassigned" pane'),
			'admin'		=> SyndLib::translate('Project administration such as adding/removing members and keywords or changing permissions'),
			);
	}
	
	function getDefinedRoles() {
		return SyndLib::array_merge_assoc(array(
			'node.role_member.Member' => SyndNodeLib::getInstance('role_member.Member')),
			parent::getDefinedRoles());
	}
	
	function isDefaultInherited($perm) {
		if ('read' == $perm)
			return false;
		return true;
	}

	/**
	 * Should receipts be sent when creating new issues
	 * @return	bool
	 */
	function getReceiptFlag() {
		return !empty($this->data['FLAG_RECEIPT']);
	}
	
	/**
	 * Should spam be filtered for this project
	 * @return	bool
	 */
	function isDiscardSpam() {
		return !empty($this->data['FLAG_DISCARD_SPAM']);
	}

	/**
	 * Should logged in user be exposed as sender of outgoing email
	 * @return	bool
	 */
	function isDisplaySender() {
		return !empty($this->data['FLAG_DISPLAY_SENDER']);
	}

	/**
	 * Should issue number be exposed in sender of outgoing email
	 * @return	bool
	 */
	function isIssueSender() {
		return false;
//		return !empty($this->data['FLAG_ISSUE_SENDER']);
	}

	function toString() {
		if (null != $this->data['INFO_HEAD'])
			return $this->data['INFO_HEAD'];
		return SyndLib::translate('No title');
	}
	
	function getHandler() {
		return 'issue';
	}

	function appendChild($child) {
		if (($child instanceof synd_node_project))
			$this->_projects->append($child);
		return parent::appendChild($child);
	}

	function removeChild($child) {
		if (($child instanceof synd_node_project))
			$this->_projects->remove($child);
		return parent::removeChild($child);
	}

	function setParent($parent) {
		parent::setParent($parent);
		$this->data['PARENT_NODE_ID'] = $parent->nodeId();
		
		// Inherit flags from parent
		if ($this->isNew()) {
			$this->data['FLAG_DISPLAY_SENDER'] = (int)!empty($parent->data['FLAG_DISPLAY_SENDER']);
			$this->data['FLAG_ISSUE_SENDER'] = (int)!empty($parent->data['FLAG_ISSUE_SENDER']);
			$this->data['FLAG_DISCARD_SPAM'] = (int)!empty($parent->data['FLAG_DISCARD_SPAM']);
		}

		// Inherit GDPR cleanup timeframe unless overriden on this project
		if (empty($this->data['INFO_CLEANUP_CUTOFF']))
			$this->data['INFO_CLEANUP_CUTOFF_SECONDS'] = $parent->data['INFO_CLEANUP_CUTOFF_SECONDS'];
		
		$this->_save();
	}

	function getParent() {
		if (null === ($parent = SyndNodeLib::getInstance($this->data['PARENT_NODE_ID'])))
			$parent = parent::getParent();
		return $parent;
	}

	function getParentOptions() {
		global $synd_user;
		$options = Module::getInstance('issue')->getProjectOptions($synd_user);
		
		$parent = $this->getParent();
		if (!isset($options[$parent->nodeId]) && !$parent->isNull() && !$parent->isPermitted('read'))
			$options[$parent->nodeId] = $parent;
		
		return $options;
	}
	
	function getChildren() {
		return $this->_projects;
	}
	
	/**
	 * Returns child projects
	 * @return	EntityCollection
	 */
	function getProjects() {
		return $this->_projects;
	}
	
	/**
	 * Returns visible child projects
	 * @return	Iterator
	 */
	function getPublishedProjects() {
		$query = self::getEntityQuery($this->_storage);
		$query->where('parent_node_id = '.$this->_db->quote($this->nodeId));
		$query->where('flag_archive = 0');
		return new DatabaseEntityCollection($this->_storage, $query);
	}
	
	/**
	 * Returns archived child projects
	 * @return	EntityCollection
	 */
	function getArchivedProjects() {
		$query = self::getEntityQuery($this->_storage);
		$query->where('parent_node_id = '.$this->_db->quote($this->nodeId));
		$query->where('flag_archive = 1');
		return new DatabaseEntityCollection($this->_storage, $query);
	}

	function getAttribute($key) {
		if (null === $this->_attributes && null != $this->data['DATA_ATTRIBUTES'])
			$this->_attributes = unserialize($this->_db->unwrapBinary($this->data['DATA_ATTRIBUTES']));
		return isset($this->_attributes[$key]) ? $this->_attributes[$key] : parent::getAttribute($key);
	}
	
	function setAttribute($key, $value) {
		if (null === $this->_attributes && null != $this->data['DATA_ATTRIBUTES'])
			$this->_attributes = unserialize($this->_db->unwrapBinary($this->data['DATA_ATTRIBUTES']));
		if (null != $value)
			$this->_attributes[$key] = $value;
		else if (isset($this->_attributes[$key]))
			unset($this->_attributes[$key]);
	}
	
	function getCreator() {
		if (null === ($creator = SyndNodeLib::getInstance($this->data['CREATE_NODE_ID'])))
			$creator = SyndNodeLib::getInstance('user_null.null');
		return $creator;
	}
	
	/**
	 * Returns the branch to this node indexed by nodeId
	 * @return	array
	 */
	function getBranch() {
		profiler_enter(__METHOD__);

		$parent = $this->getParent();
		$branch = $parent->isNull() ? array() : $parent->getBranch();
		if (!isset($branch[$this->nodeId]))
			$branch[$this->nodeId] = $this;

		profiler_leave(__METHOD__);
		return $branch;
	}
	
	function getTitle() {
		return $this->data['INFO_HEAD'];
	}
	
	function setTitle($title) {
		$this->data['INFO_HEAD'] = $title;
		$this->_save();
	}
	
	function getDescription() {
		return $this->data['INFO_DESC'];
	}
	
	function setDescription($description) {
		$this->data['INFO_DESC'] = $description;
		$this->_save();
	}
	
	function getEmail() {
		if (!empty($this->data['INFO_EMAIL']))
			return $this->data['INFO_EMAIL'];
		if (null !== ($project = $this->getAbsoluteEmail()) && !$project->isNull() && null != $this->data['INFO_PROJECT_ID']) {
			if ($this->isIssueSender()) {
				return preg_replace('/(?<=.)(\+.*)?@/', '+'.$this->getProjectId().'@', $project->data['INFO_EMAIL']);
			}
			
			return $project->data['INFO_EMAIL'];
		}
		return null;
	}
	
	function getEmailSubjectTag() {
		if (null != ($namespace = Module::getInstance('issue')->getNamespace())) {
			return '#'.$this->getProjectId().'@'.$namespace;
		}
		return null;
	}
	
	function setEmail($email) {
		$this->data['INFO_EMAIL'] = $email;
	}
	
	/**
	 * Returns the specified email address for this project branch
	 * @return	synd_node_project	Returns null on failure
	 */
	function getAbsoluteEmail() {
		if (!empty($this->data['INFO_EMAIL']))
			return $this;
		$parent = $this->getParent();
		if (!$parent->isNull())
			return $parent->getAbsoluteEmail();
		return $parent;
	}

	/**
	 * Returns the project identifier or node id.
	 * @return	string
	 */
	function getProjectId() {
		return null != $this->data['INFO_PROJECT_ID'] ? strtolower($this->data['INFO_PROJECT_ID']) : $this->nodeId;
	}
	
	/**
	 * Returns the project shortcode
	 * @return	string
	 */
	function getProjectCode() {
		return substr(null != $this->data['INFO_PROJECT_ID'] ? 
			$this->data['INFO_PROJECT_ID'] : $this->toString(),0,6);
	}

	/**
	 * Returns the GDPR cleanup timeframe in effect for this project
	 * @return	int
	 */
	function getCleanupTimeframe() {
		if (!empty($this->data['INFO_CLEANUP_CUTOFF']))
			return $this->data['INFO_CLEANUP_CUTOFF'];
		if (!$this->getParent()->isNull())
			return $this->getParent()->getCleanupTimeframe();
		return '-5 years';
	}

	/**
	 * Sets the GDPR cleanup timeframe for this project and its children
	 */
	function setCleanupTimeframe($timeframe, $ischild=false) {
		if (!$ischild)
			$this->data['INFO_CLEANUP_CUTOFF'] = $timeframe;
		$this->data['INFO_CLEANUP_CUTOFF_SECONDS'] = time() - strtotime($this->getCleanupTimeframe());

		// Recursively set for child projects that haven't got a timeframe specified
		if (!empty($timeframe)) {
			foreach ($this->getProjects() as $project) {
				if (empty($project->data['INFO_CLEANUP_CUTOFF']))
					$project->setCleanupTimeframe($timeframe, true);
			}
		}

		$this->_save();
	}
	
	/**
	 * Returns the node ids for this entire project branch
	 * @return	array
	 */
	function getProjectBranchIds() {
		$ids = array($this->nodeId);
		foreach ($this->getProjects() as $project)
			$ids = array_merge($ids, $project->getProjectBranchIds());
		return $ids;
	}
	
	/**
	 * Returns the collection of issues in this project and any sub projects
	 * @return	Collection
	 */
	function getCollection() {
		if (null === $this->_collection) {
			$sql = "
				SELECT i.node_id FROM synd_issue i
				WHERE 
					i.parent_node_id = ".$this->_db->quote($this->nodeId)." AND
					i.info_status >= 0 AND i.info_status < ".synd_node_issue::CLOSED;
			$fragments = SyndLib::invoke($this->getProjects(), 'getCollection');
			$fragments[] = SyndType::factory('query_collection', $this->_storage, $sql);
			$this->_collection = SyndType::factory('composite_collection', $fragments);
		}
		return $this->_collection;
	}
	
	/**
	 * @see		Collection::getCount()
	 */
	function getCount() {
		$collection = $this->getCollection();
		return $collection->getCount();
	}
	
	/**
	 * @see		Collection::getContents()
	 */
	function getContents($offset = 0, $limit = null) {
		$collection = $this->getCollection();
		return $collection->getContents($offset, $limit);
	}
	
	/**
	 * @see		Collection::getFilteredCount()
	 */
	function getFilteredCount($classes) {
		$collection = $this->getCollection();
		return $collection->getFilteredCount($classes);
	}

	/**
	 * @see		Collection::getFilteredContents()
	 */
	function getFilteredContents($classes) {
		$collection = $this->getCollection();
		return $collection->getFilteredContents($classes);
	}

	/**
	 * Returns the MailNotifier helper for this node
	 * @access	protected
	 * @return	synd_type_issue_mail_notifier
	 */
	function getMailNotifier() {
		if (null == $this->_mailNotifier) {
			if (null != $this->data['DATA_NOTIFIER'])
				$this->_mailNotifier = unserialize($this->_db->unwrapBinary($this->data['DATA_NOTIFIER']));
			else {
				$this->_mailNotifier = SyndType::factory('mail_notifier');
				$this->_mailNotifier->setParent($this);
				$this->_save();
			}
		}
		return $this->_mailNotifier;
	}
	
	function getDefaultClient() {
		if (empty($this->data['INFO_DEFAULT_CLIENT'])) 
			$client = SyndNodeLib::getInstance('user_null.null');
		else {
			$module = Module::getInstance('user');
			if (null === ($client = $module->resolveLogin($this->data['INFO_DEFAULT_CLIENT'])))
				$client = SyndNodeLib::getInstance('user_case.'.$this->data['INFO_DEFAULT_CLIENT']);
		}
		return $client;
	}
	
	function getDefaultResolveBy() {
		return strtotime($this->data['INFO_DEFAULT_RESOLVE']);
	}
	
	function getMinimumReopenBy($ts) {
		return strtotime($this->data['INFO_DEFAULT_REOPEN'], $ts);
	}
	
	function getIndexDocument($index) {
		return $index->createFragment($this->getProjectId().' '.$this->data['INFO_HEAD'].' '.$this->data['INFO_DESC']);
	}
	
	function addMember($user) {
		$this->_members->append($user);
	}
	
	function removeMember($user) {
		if ($this->isLocalMember($user))
			$this->_members->remove($user);
		else if ($this->getParent()->isNull())
			$this->getParent()->removeMember($user);
	}
	
	function isMember($user) {
		if ($this->_members->exists($user))
			return true;
		if (empty($this->data['FLAG_INHERIT_MEMBERS'])) 
			return false;
		$parent = $this->getParent();
		return !$parent->isNull() && $parent->isMember($user);
	}
	
	function isLocalMember($user) {
		return $this->_members->exists($user);
	}

	function getMembers() {
		if (!empty($this->data['FLAG_INHERIT_MEMBERS']) && !$this->getParent()->isNull())
			return new CompositeDomainObjectCollection(array($this->getParent()->getMembers(), $this->_members));
		return $this->_members;
	}

	function getResources() {
		$sql = "
			SELECT r.node_id FROM synd_plan_resource r
			WHERE r.parent_node_id = ".$this->_db->quote($this->nodeId);
		return $this->_storage->getInstances($this->_db->getCol($sql));
	}

	function getCategories() {
		if (!empty($this->data['FLAG_INHERIT_CATEGORIES']) && null !== ($parent = $this->getParent()) && !$parent->isNull())
			return SyndLib::array_merge_assoc($this->getLocalCategories(), $parent->getCategories());
		return $this->getLocalCategories();
	}
	
	function getCategoriesRecursive(MatchDecider $filter = null) {
		if (null === $filter)
			$filter = new BooleanDecider();
	
		$result = array();
		foreach ($this->getCategories() as $category) {
			if ($filter->accept($category)) {
				$result[$category->nodeId] = $category;
				$result = array_merge($result, $category->getCategoriesRecursive($filter));
			}
		}
		
		return $result;
	}
	
	function getLocalCategories() {
		if (null === $this->_keywords) {
			$sql = "
				SELECT k.node_id FROM synd_keyword k
				WHERE k.parent_node_id = ".$this->_db->quote($this->nodeId);
			$this->_keywords = (array)$this->_db->getCol($sql);
			$this->_node_onchange_internal();
		}
		return $this->_storage->getInstances($this->_keywords);
	}
	
	/**
	 * Returns the defined and inherited time periods
	 * @return	array
	 */
	function getPeriods() {
		$sql = "
			SELECT pe.node_id FROM synd_plan_period pe
			WHERE pe.parent_node_id = ".$this->_db->quote($this->nodeId);
		$result = $this->_storage->getInstances($this->_db->getCol($sql));
		
		$parent = $this->getParent();
		if (!$parent->isNull()) {
			foreach (array_keys($periods = $parent->getPeriods()) as $key)
				$result[$key] = $periods[$key];
		}
		
		return $result;
	}

	/**
	 * Returns the currently selected time period
	 * @return	synd_node_period
	 */
	function getSelectedPeriod() {
		$storage = SyndNodeLib::getDefaultStorage('period');
		$periods = $this->getPeriods();
		
		if (isset($_SESSION['plan']['period'], $periods[$_SESSION['plan']['period']])) 
			$period = $periods[$_SESSION['plan']['period']];
		else if (empty($periods))
			$period = SyndNodeLib::getInstance('null.null');
		else {
			$period = $periods[reset(array_keys(SyndLib::sort($periods,'getBeginTime')))];
			$_SESSION['plan']['period'] = $period->nodeId;
		}
		
		return $period;
	}
	
	/**
	 * Returns the resource allocation for this project summarized per resource
	 * @return	PagedListing
	 */
	function getResourceAllocation() {
		require_once 'design/gui/PagedListing.class.inc';
		$query = $this->_db->createQuery();
		$r = $query->join('synd_plan_resource', 'r');
		$b = $query->join('synd_plan_budgeted', 'b');
		
		$query->where("$r.project_node_id = ".$this->_db->quote($this->nodeId));
		$query->where("$b.project_node_id = ".$this->_db->quote($this->nodeId));
		$query->column("$r.*");
		return new PagedListing($this->_storage, $query, array("$r.info_head"));
	}

	function _view_newIssue(Request $request, Template $page) {
		global $synd_user;
		if (!$this->isPermitted('read'))
			throw new ForbiddenException();
		$issue = $this->appendChild($this->_storage->factory('issue'));
		if (array_key_exists($synd_user->nodeId, $issue->getAssignedOptions()))
			$issue->setAssigned($synd_user);
		return array('redirect' => tpl_view($this->getHandler(),'edit',$issue->nodeId));
	}
	
	function _view_newKeyword(Request $request, Template $page) {
		if (!$this->isPermitted('admin'))
			throw new ForbiddenException();
		$keyword = $this->appendChild(SyndNodeLib::factory('keyword'));
		return array('redirect' => tpl_view_jump($this->getHandler(),'edit',$keyword->nodeId));
	}
	
	function bubbleEvent($event) {
		parent::bubbleEvent($event);
		
		$mailNotifier = $this->getMailNotifier();
		$mailNotifier->handleEvent($event);
		
		if (!$this->_shutdown) {
			$this->_shutdown = true;
			SyndLib::attachHook('shutdown', array($this, '_callback_shutdown'));
		}
	}

	/**
	 * Dispatches queued emails
	 */
	function _callback_shutdown() {
		if (null !== $this->_mailNotifier)
			$this->_mailNotifier->send();
	}

	function validate($data) {
		require_once 'core/lib/SyndHTML.class.inc';
		$errors = parent::validate($data);
		
		if (!empty($data['PARENT_NODE_ID'])) {
			$lastParent = $this->getParent();
			$parent = $this->_storage->getInstance($data['PARENT_NODE_ID']);
			if (null === $parent || !$parent->isPermitted('write') && $parent->nodeId != $lastParent->nodeId)
				$errors['PARENT_NODE_ID'] = SyndLib::translate('Invalid project selected.');
		}

		if (!empty($data['INFO_PROJECT_ID'])) {
			if (!preg_match('/^[\w\-]+$/', $data['INFO_PROJECT_ID']) || SyndHTML::translateNationalChars($data['INFO_PROJECT_ID']) != $data['INFO_PROJECT_ID'])
				$errors['INFO_PROJECT_ID'] = SyndLib::translate('Project id must consist only of letters and digits (no national chars)');
			else if (preg_match('/^\d+$/', $data['INFO_PROJECT_ID']))
				$errors['INFO_PROJECT_ID'] = SyndLib::translate('Project id may not consist of numbers only');
			else if (null !== ($project = synd_node_project::findProject($this->_storage, $data['INFO_PROJECT_ID'])) && $project->nodeId != $this->nodeId)
				$errors['INFO_PROJECT_ID'] = SyndLib::translate('Project id already taken.');
		}

		if (isset($data['INFO_DEFAULT_RESOLVE']) && !strtotime($data['INFO_DEFAULT_RESOLVE']))
			$errors['INFO_DEFAULT_RESOLVE'] = SyndLib::translate('Invalid resolve by offset');
		if (isset($data['INFO_DEFAULT_REOPEN']) && !strtotime($data['INFO_DEFAULT_REOPEN']))
			$errors['INFO_DEFAULT_REOPEN'] = SyndLib::translate('Invalid reopen offset');
		if (!empty($data['INFO_CLEANUP_CUTOFF'])) {
			$cutoff = strtotime($data['INFO_CLEANUP_CUTOFF']);
			$mincutoff = '-1 years';

			if (!$cutoff)
				$errors['INFO_CLEANUP_CUTOFF'] = SyndLib::translate('Invalid GDPR cleanup cutoff');
			else if ($cutoff > strtotime($mincutoff))
				$errors['INFO_CLEANUP_CUTOFF'] = SyndLib::translate("GDPR cleanup cutoff must be at least '$mincutoff'");
		}

		return $errors;
	}

	function merge($data) {
		if (isset($data['PARENT_NODE_ID']) && null == $data['PARENT_NODE_ID'])
			$data['PARENT_NODE_ID'] = 'null.issue';
		
		if (isset($data['INFO_HEAD'])) {
			$data['FLAG_HIDE_ISSUES'] = (int)!empty($data['FLAG_HIDE_ISSUES']);
			$data['FLAG_INHERIT_MEMBERS'] = (int)!empty($data['FLAG_INHERIT_MEMBERS']);
			$data['FLAG_INHERIT_CATEGORIES'] = (int)!empty($data['FLAG_INHERIT_CATEGORIES']);
			$data['FLAG_DISPLAY_SENDER'] = (int)!empty($data['FLAG_DISPLAY_SENDER']);
			$data['FLAG_ISSUE_SENDER'] = (int)!empty($data['FLAG_ISSUE_SENDER']);
			$data['FLAG_DISCARD_SPAM'] = (int)!empty($data['FLAG_DISCARD_SPAM']);
			$data['FLAG_RECEIPT'] = (int)!empty($data['FLAG_RECEIPT']);
			$data['FLAG_ARCHIVE'] = (int)!empty($data['FLAG_ARCHIVE']);

			$this->setCleanupTimeframe($data['INFO_CLEANUP_CUTOFF']);
		}
		
		if (!empty($data['attributes'])) {
			foreach ((array)$data['attributes'] as $key => $value)
				$this->setAttribute($key, $value);
		}
		
		if (!empty($data['templates'])) {
			foreach ($data['templates'] as $id => $template) {
				foreach ($template as $locale => $content)
					$this->setTemplate($id, $locale, $content);
			}
		}
		
		parent::merge($data);
	}
	
	/**
	 * Specifies a template to use for an event
	 * @param	string	Name of template
	 * @param	string	Template locale {'en', 'sv', ..}
	 * @param	string	Template contents
	 */
	function setTemplate($name, $locale, $contents) {
		$this->_templates[$name][$locale] = $contents;
		$this->_node_onchange_internal();
	}
	
	/**
	 * Returns the template contents to use for an event
	 * @param	string	Name of template
	 * @param	string	Template locale {'en', 'sv', ..}
	 * @return	string
	 */
	function getTemplate(Template $page, $id, $locale = 'en') {
		if (!isset($this->_templates[$id][$locale])) {
			$sql = "
				SELECT t.data_contents FROM synd_project_template t
				WHERE 
					t.project_node_id = ".$this->_db->quote($this->nodeId)." AND
					t.template_id = ".$this->_db->quote($id)." AND
					t.locale = ".$this->_db->quote($locale);
			$this->_templates[$id][$locale] = (string)$this->_db->getOne($sql);
			$this->_node_onchange_internal();
		}
		
		if (null == $this->_templates[$id][$locale]) {
			$parent = $this->getParent();
			if (!$parent->isNull())
				return $parent->getTemplate($page, $id, $locale);
			else if (null == ($this->_templates[$id][$locale] = $page->fetchnode($this,"templates/$id.tpl",null,false)))
				$this->_templates[$id][$locale] = $page->fetchnode($this,"templates/onchange.tpl");
		}
		
		return $this->_templates[$id][$locale];
	}

	/**
	 * Removes a template thereby resetting it to the default one
	 * @param	string	Id of template
	 * @param	string	Template locale
	 */
	function _view_delTemplate(Request $request, Template $page) {
		if (!$this->isPermitted('admin'))
			throw new ForbiddenException();
		if (!isset($request[2], $request[3]))
			return;
		if (!isset($request['confirm']))
			return array('content' => $page->fetchnode($this,'confirm_remove_template.tpl',array('template'=>$request[2],'locale'=>$request[3])));
		
		$sql = "
			DELETE FROM synd_project_template
			WHERE 
				project_node_id = ".$this->_db->quote($this->nodeId)." AND
				template_id = ".$this->_db->quote($request[2])." AND
				locale = ".$this->_db->quote($request[3]);

		$this->_db->query($sql);
		if (isset($this->_templates[$request[2]][$request[3]]))
			unset($this->_templates[$request[2]][$request[3]]);
		$this->_node_onchange_internal();
	}

	/**
	 * Child project factory
	 */
	function _view_newProject(Request $request, Template $page) {
		if (!$this->isPermitted('write'))
			throw new ForbiddenException();
		$project = $this->appendChild(SyndNodeLib::factory('project'));
		return array('redirect' => tpl_view_jump($project->getHandler(),'edit',$project->nodeId));
	}

	/**
	 * Searches for potential new members
	 * @param	string	Query to search for
	 * @return	Collection
	 */
	function findMemberInstances($query) {
		$module = Module::getInstance('user');
		return $module->findUsers($query);
	}

	function _view_addMembers(Request $request, Template $page) {
		if (!$this->isPermitted('admin'))
			throw new ForbiddenException();
		if (empty($request['users']) || !is_array($request['users']))
			return null;
		foreach (SyndNodeLib::getInstances($request['users']) as $user)
			$this->addMember($user);
		$this->save();
	}
	
	function _view_removeMember(Request $request, Template $page) {
		if (!$this->isPermitted('admin'))
			throw new ForbiddenException();
		if (null != ($user = SyndNodeLib::getInstance($request['user']))) {
			if (empty($request['confirm']))
				return array('content' => $page->fetchnode($this,'confirm_remove_member.tpl',array('user'=>$user)));
			$this->removeMember($user);
			$this->save();
		}
	}

	function _view_saveNotification(Request $request, Template $page) {
		global $synd_user;
		if (!$this->isPermitted('write'))
			throw new ForbiddenException();
		
		$mailNotifier = $this->getMailNotifier();
		if (!empty($request[2]) && null !== ($listener = SyndLib::getInstance($request[2]))) {
			$events = empty($request['events']) ? array() : $request['events'];
			foreach (array_diff($mailNotifier->getListenerEvents($listener),$events) as $event)
				$mailNotifier->removeListener($event, $listener);
			foreach (array_diff($events,$mailNotifier->getListenerEvents($listener)) as $event)
				$mailNotifier->addListener($event, $listener);
		}

		if (!empty($request['event']) && !empty($request['email']) && null !== ($module = Module::getInstance('user'))) {
			if (!$this->isPermitted('admin'))
				throw new ForbiddenException();
			if (null === ($listener = $module->resolveEmail($request['email'])))
				$listener = SyndNodeLib::getInstance('user_case.'.trim($request['email']," \t\r\n\""));
			$mailNotifier->addListener($request['event'], $listener);
		}
		
		$this->_save();
	}
	
	function _view_removeNotification(Request $request, Template $page) {
		if (!$this->isPermitted('admin'))
			throw new ForbiddenException();
		if (null !== ($listener = SyndLib::getInstance($request[3]))) {
			if (empty($request['confirm']))
				return array('content' => $page->fetchnode($this,'confirm_remove_notification.tpl',array('event'=>$request[2],'listener'=>$listener)));
			$mailNotifier = $this->getMailNotifier();
			$mailNotifier->removeListener($request[2], $listener);
			$this->_save();
		}
	}

	/**
	 * Autosuggest clients callback
	 * @see		synd_module_user::findSuggestedUsers()
	 */
	function _public_findSuggestedUsers($query) {
		if (!$this->isPermitted('write'))
			throw new ForbiddenException();
		ignore_user_abort(false);
		$module = Module::getInstance('issue');
		return $module->findSuggestedUsers($query);
	}

	/**
	 * Builds the context menus
	 * @param	string	Selected project id
	 * @return	array
	 */
	function _public_getContextMenu($selected) {
		$projects = new MethodFilterIterator($this->getPublishedProjects()->getIterator(), 'isPermitted', 'read');
		synd_node_issue::treeContextOptions($result, SyndLib::sort(iterator_to_array($projects)), $selected);
		return $result;
	}

	/**
	 * Attempts to find a project given an id
	 * @param	DomainStorage		The storage to search
	 * @param	integer				The issue id
	 * @return	synd_node_project	Returns null if nothing found
	 */
	static function findProject($storage, $id) {
		$persistent = $storage->getPersistentStorage();
		$database = $persistent->getDatabase();
		
		$sql = "
			SELECT p.node_id FROM synd_project p
			WHERE LOWER(p.info_project_id) = ".$database->quote(strtolower($id));
		if (null !== ($project = $storage->getInstance($database->getOne($sql))))
			return $project;
			
		$result = null;
		return $result;
	}
	
	/**
	 * Adds an query->user mapping, for example mapping an external email to a known user
	 * @param	User	User to map to
	 * @param	string	Query to map from
	 */
	function addCustomerMapping($customer, $query) {
		$this->_db->replace('synd_project_mapping', array(
			'PROJECT_NODE_ID'	=> $this->nodeId,
			'CUSTOMER_NODE_ID'	=> $customer->id(),
			'QUERY'				=> $query));
	}
	
	function _view_addCustomerMappings(Request $request, Template $page) {
		if (!$this->isPermitted('write'))
			throw new ForbiddenException();
		
		if (isset($request['mappings'])) {
			$module = Module::getInstance('user');
			
			foreach (explode("\n", $request['mappings']) as $line) {
				if (count($fields = preg_split('/\s+/', trim($line), 3)) >= 2 &&
					(null != ($user = $module->resolveLogin($fields[0])) || null != ($user = $module->resolveEmail($fields[0]))))
					$this->addCustomerMapping($user, $fields[1]);
			}
		}
	}
	
	function _view_delCustomerMapping(Request $request, Template $page) {
		if (!$this->isPermitted('write'))
			throw new ForbiddenException();
		
		if (isset($request['customer_node_id'], $request['query'])) {
			$sql = "
				DELETE FROM synd_project_mapping
				WHERE 
					project_node_id = ".$this->_db->quote($this->nodeId)." AND
					customer_node_id = ".$this->_db->quote($request['customer_node_id'])." AND
					query = ".$this->_db->quote($request['query']);
			$this->_db->query($sql);
		}
	}

	function _public_getAssignedOptions() {
		if (!$this->isPermitted('read'))
			throw new ForbiddenException();

		$options = array();
		if ($this->isPermitted('write')) {
			foreach ($this->getMembers() as $member)
				$options[$member->nodeId] = $member->toString().' ('.$member->getContact().')';
		}

		uasort($options, 'strcasecmp');
		return $options;
	}
	
	function _node_onchange_child() {
		$this->_collection = null;
		$this->_keywords = null;
		parent::_node_onchange_child();
	}

	/**
	 * Dispatch emails and serialize notifier and attributes
	 */
	function _entity_onpreflush() {
		global $synd_user;
		parent::_entity_onpreflush();

		$this->data['UPDATE_NODE_ID'] = $synd_user->nodeId;
		$this->data['TS_UPDATE'] = time();

		if (null !== $this->_mailNotifier) 
			$this->data['DATA_NOTIFIER'] = $this->_db->wrapBinary(serialize($this->_mailNotifier));
		if (null !== $this->_attributes) 
			$this->data['DATA_ATTRIBUTES'] = empty($this->_attributes) ? null : $this->_db->wrapBinary(serialize($this->_attributes));
	}
	
	/**
	 * Store changed templates
	 */
	function _entity_onpostflush() {
		global $synd_user;
		if ($this->isNew() && !$this->isPermitted('admin'))
			SyndLib::runHook('grant', $this, $synd_user, 'admin');
		
		parent::_entity_onpostflush();
		
		if (!empty($this->_templates)) {
			$parent = $this->getParent();
			
			foreach ($this->_templates as $id => $templates) {
				foreach ($templates as $locale => $contents) {
					$this->_db->replace('synd_project_template', array(
						'PROJECT_NODE_ID' => $this->nodeId,
						'TEMPLATE_ID' => $id,
						'LOCALE' => $locale,
						'DATA_CONTENTS' => $contents));
				}
			}
		}

		$this->_projects->flush();
		$this->_members->flush();
	}
	
	function _node_onpredelete() {
		// Attempt to move issues to parent
		$parent = $this->getParent();
		if ($parent->isNull() && $this->_db->getOne("SELECT 1 FROM synd_issue i WHERE i.parent_node_id = ".$this->_db->quote($this->nodeId))) {
			trigger_error('Top-level projects must be empty of issues before deletion', E_USER_WARNING);
			return true;
		}
		
		$timelimit = ini_get('max_execution_time');
		set_time_limit(0);
		$this->_db->begin();
		
		$sql = "
			SELECT i.NODE_ID FROM synd_issue i
			WHERE i.parent_node_id = ".$this->_db->quote($this->nodeId);
		$rows = $this->_db->query($sql);
		$ids = array();
		
		for ($rows->rewind(); $rows->valid(); ) {
			for ($i=0; $i<250 && $rows->valid(); $i++, $rows->next()) {
				$current = $rows->current();
				$ids[] = $current['NODE_ID'];
			}
			foreach ($this->_storage->getInstances($ids) as $issue) {
				$parent->appendChild($issue);
				$issue->save();
			}
			$this->_storage->flush();
		}
		
		$this->_db->commit();
		set_time_limit($timelimit);
		
		return parent::_node_onpredelete();
	}
}
