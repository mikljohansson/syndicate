<?php
require_once 'core/model/DatabaseEntity.class.inc';

class synd_node_keyword extends AbstractDatabaseEntity {
	/**
	 * Child projects
	 * @var		AggregateEntityCollection
	 */
	protected $_keywords = null;

	function __construct($storage, $database, $data) {
		parent::__construct($storage, $database, $data);
		$this->_keywords = $this->aggregate(__CLASS__);
	}

	function __factory() {
		$this->data['NODE_ID'] = $this->classId().'.'.$this->_db->nextId('synd_keyword');
		parent::__factory();
	}
	
	function __clone() {
		$this->data['NODE_ID'] = $this->classId().'.'.$this->_db->nextId('synd_keyword');
		$this->_projects = $this->aggregate(__CLASS__);
		parent::__clone();
	}
	
	static function getEntityQuery(DomainStorage $storage) {
		return self::createQuery($storage, 'synd_keyword');
	}

	function nodeId() {
		return $this->data['NODE_ID'];
	}
	
	function hasPermission($user, $perm, $arg = null) {
		if ('read' == $perm && null != $arg)
			return false;
		if ('merge' == $perm && in_array($arg, array(
			'PARENT_NODE_ID','INFO_HEAD','INFO_DESC', 'FLAG_MANDATORY', 'FLAG_SINGLESELECT')))
			return $this->hasPermission($user, 'write');
		return parent::hasPermission($user, $perm, $arg);
	}

	function merge($data) {
		if (isset($data['INFO_HEAD'])) {
			$data['FLAG_MANDATORY'] = (int)!empty($data['FLAG_MANDATORY']);
			$data['FLAG_SINGLESELECT'] = (int)!empty($data['FLAG_SINGLESELECT']);
		}
		
		parent::merge($data);
	}

	function setParent($parent) {
		$this->data['PARENT_NODE_ID'] = $parent->nodeId;
		$this->_save();
	}
	
	function getParent() {
		if (null === ($parent = SyndNodeLib::getInstance($this->data['PARENT_NODE_ID'])))
			$parent = parent::getParent();
		return $parent;
	}

	/**
	 * Returns projects and keywords possible to append this keyword to
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
	 * Returns child categories
	 * @return	EntityCollection
	 */
	function getCategories() {
		return $this->_keywords;
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
	
	function getHandler() {
		return 'issue';
	}
	
	function toString() {
		return $this->data['INFO_HEAD'];
	}
	
	function setTitle($title) {
		$this->data['INFO_HEAD'] = $title;
		$this->_save();
	}

	function getDescription() {
		return $this->data['INFO_DESC'];
	}
	
	function acceptIndexer(IndexBuilder $indexer) {
		parent::acceptIndexer($indexer);
		$indexer->visitFragment($this->data['INFO_HEAD']);
		$indexer->visitFragment($this->data['INFO_DESC']);
	}

	function appendChild($child) {
		if (($child instanceof synd_node_keyword))
			$this->_keywords->append($child);
		return parent::appendChild($child);
	}

	function removeChild($child) {
		if (($child instanceof synd_node_keyword))
			$this->_keywords->remove($child);
		return parent::removeChild($child);
	}

	function isSelectable() {
		return count($this->getCategories()) == 0;
	}
	
	function isMandatory() {
		return $this->data['FLAG_MANDATORY'];
	}

	function isSingleSelect() {
		if ($this->data['FLAG_SINGLESELECT']) {
			return true;
		}
		
		$project = $this->getParent();
		if ($project instanceof synd_node_keyword) {
			return $project->isSingleSelect();
		}
		
		return false;
	}

	/**
	 * Builds the context menus
	 * @param	string	Selected project id
	 * @return	array
	 */
	function _public_getContextMenu($selected) {
		$projects = new MethodFilterIterator($this->getCategories()->getIterator(), 'isPermitted', 'read');
		synd_node_issue::treeCategoryContextOptions($result, SyndLib::sort(iterator_to_array($projects)), $selected);
		return $result;
	}

	function _view_newKeyword(Request $request, Template $page) {
		if (!$this->isPermitted('admin'))
			throw new ForbiddenException();
		$keyword = $this->appendChild(SyndNodeLib::factory('keyword'));
		return array('redirect' => tpl_view_jump($this->getHandler(),'edit',$keyword->nodeId));
	}

	function _node_onchange_child() {
		$this->_keywords->flush();
		parent::_node_onchange_child();
	}

	function _entity_onpostflush() {
		parent::_entity_onpostflush();
		$this->_keywords->flush();
	}
}
