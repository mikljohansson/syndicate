<?php
require_once 'core/model/DatabaseEntity.class.inc';
require_once 'core/model/workflow/Workflow.class.inc';
require_once 'core/model/workflow/WorkflowActivity.class.inc';

class synd_node_workflow extends AbstractDatabaseEntity implements Workflow {
	protected $_events = null;
	protected $_removed = null;
	
	function __construct($storage, $database, $data) {
		if (null != $data['DATA_ACTIVITY'])
			$data['DATA_ACTIVITY'] = unserialize($database->unwrapBinary($data['DATA_ACTIVITY']));
		parent::__construct($storage, $database, $data);
		
		$sql = "
			SELECT e.event FROM synd_workflow_event e
			WHERE e.parent_node_id = ".$this->_db->quote($this->nodeId);
		$this->_events = (array)$this->_db->getCol($sql);
	}
	
	function __factory() {
		global $synd_user;
		$this->data['NODE_ID'] = $this->classId().'.'.$this->_db->nextId('synd_workflow');
		$this->data['TS_CREATE'] = time();
		$this->data['CREATE_NODE_ID'] = $this->data['UPDATE_NODE_ID'] = $synd_user->nodeId;
		$this->data['DATA_ACTIVITY'] = new synd_workflow_sequence();
		parent::__factory();
	}

	function __clone() {
		$this->data['NODE_ID'] = $this->classId().'.'.$this->_db->nextId('synd_workflow');
		if (null != $this->data['DATA_ACTIVITY'])
			$this->data['DATA_ACTIVITY'] = clone $this->data['DATA_ACTIVITY'];
		parent::__clone();
	}
	
	static function getEntityQuery(DomainStorage $storage) {
		return self::createQuery($storage, 'synd_workflow');
	}

	function nodeId() {
		return $this->data['NODE_ID'];
	}

	function toString() {
		return $this->data['INFO_HEAD'];
	}
	
	function getDescription() {
		return $this->data['INFO_DESC'];
	}
	
	function getHandler() {
		return $this->getParent()->getHandler();
	}
	
	function getParent() {
		if (null === ($parent = $this->_storage->getInstance($this->data['PARENT_NODE_ID'])))
			$parent = parent::getParent();
		return $parent;
	}

	function setParent($parent) {
		$this->data['PARENT_NODE_ID'] = $parent->nodeId;
		$this->_save();
	}
	
	function getParentOptions() {
		global $synd_user;
		$options = Module::getInstance('issue')->getProjectOptions($synd_user);
		
		$parent = $this->getParent();
		if (!isset($options[$parent->nodeId]) && !$parent->isNull() && !$parent->isPermitted('read'))
			$options[$parent->nodeId] = $parent;
		
		return $options;
	}
	
	function getEventOptions() {
		return (array)SyndLib::runHook('workflow_event_protos');
	}
	
	function getEvents() {
		return $this->_events;
	}
	
	function addListener(WorkflowActivity $activity, DomainObject $context, $interface, $events) {
	
	}

	function process($subjects, Response $response) {
		$this->getActivity()->process($this, $subjects, $response);
	}
	
	function handleEvent(DomainEvent $event) {
		$this->getActivity()->process($this, $event->getSubject());
	}
	
	/**
	 * Assigns the entry point activity
	 * @param	WorkflowActivity	Activity to delegate events to
	 */
	function setActivity(WorkflowActivity $activity) {
		$this->data['DATA_ACTIVITY'] = $activity;
	}
	
	/**
	 * Returns the entry point activity
	 * @return	WorkflowActivity
	 */
	function getActivity() {
		return $this->data['DATA_ACTIVITY'];
	}
	
	/**
	 * Attach an event interface to listen to
	 * @param	string	Event interface name
	 */
	function attachEvent($event) {
		if (false !== ($key = array_search($event, (array)$this->_removed)))
			unset($this->_removed[$key]);
		$this->_events[] = $event;
	}
	
	/**
	 * Does this workflow have any registered listeners
	 * @return	bool
	 */
	function isEmpty() {
		return empty($this->_listeners);
	}
	
	/**
	 * Is this workflow a prototype
	 * @return	bool
	 */
	function isPrototype() {
		return (bool)$this->data['FLAG_PROTOTYPE'];
	}
	
	/**
	 * Sets the prototype status
	 * @param	bool	Is prototype flag
	 */
	function setPrototype($prototype) {
		$this->data['FLAG_PROTOTYPE'] = (int)$prototype;
	}
	
	function getCompositeData() {
		$data = parent::getCompositeData();
		$data['events'] = $this->getEvents();
		return $data;
	}

	function merge($data) {
		$data['FLAG_CONTEXT_MENU'] = (int)!empty($data['FLAG_CONTEXT_MENU']);
		$data['FLAG_SIDEBAR_MENU'] = (int)!empty($data['FLAG_SIDEBAR_MENU']);
		$this->_events = isset($data['events']) ? (array)$data['events'] : array();
		parent::merge($data);
	}
	
	function _view_edit(Request $request, Template $page) {
		$this->checkout();
		return parent::_view_edit($request, $page);
	}
	
	function _entity_onpreflush() {
		global $synd_user;
		parent::_entity_onpreflush();

		if (!$synd_user->isNull())
			$this->data['UPDATE_NODE_ID'] = $synd_user->nodeId;
		$this->data['TS_UPDATE'] = time();
	}

	function _entity_onpostflush() {
		$sql = "
			DELETE FROM synd_workflow_event
			WHERE parent_node_id = ".$this->_db->quote($this->nodeId);
		$this->_db->query($sql);

		foreach ($this->_events as $event) {
			$this->_db->insert('synd_workflow_event', array(
				'PARENT_NODE_ID'	=> $this->nodeId,
				'EVENT'				=> $event));
		}
	}
}
