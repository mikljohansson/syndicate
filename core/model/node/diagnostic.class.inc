<?php
require_once 'core/model/node/page.class.inc';
require_once 'core/model/node/case.class.inc';

class synd_node_diagnostic extends synd_node_page {
	var $_children = null;

	function hasPermission($user, $perm, $arg = null) {
		if ('append' == $perm && $this->hasPermission($user, 'write'))
			return ($arg instanceof synd_node_question) || ($arg instanceof synd_node_relation);
		return parent::hasPermission($user, $perm, $arg);
	}

	function insertBefore($child, $before = null) {
		if ($child->isNew())
			return false;

		if (($child instanceof synd_node_relation))
			$child = $child->getChild();
		
		// Check for existing relation
		$prevRelation = null;
		foreach (array_keys($relations = $this->getChildren()) as $key) {
			$existing = $relations[$key]->getChild();
			if ($existing->nodeId == $child->nodeId)
				$prevRelation = $relations[$key];
		}
		
		$relation = SyndNodeLib::factory('relation');
		$relation->setParent($this);
		$relation->setChild($child);
		
		if (false !== ($relation = parent::insertBefore($relation, $before))) {
			if (null != $prevRelation)
				$prevRelation->delete();
			$relation->save();
		}
		
		return $relation;
	}

	function getChildren() {
		if (null === $this->_children) {
			$sql = "
				SELECT r.node_id, r.info_weight FROM synd_relation r
				WHERE r.parent_node_id = ".$this->_db->quote($this->nodeId)."
				UNION
				SELECT c.node_id, c.info_weight FROM synd_content c
				WHERE c.parent_node_id = ".$this->_db->quote($this->nodeId)."
				ORDER BY info_weight";
			$this->_children = (array)$this->_db->getCol($sql);
			$this->_node_onchange_internal();
		}
		return $this->_storage->getInstances($this->_children);
	}
	
	function getQuestions() {
		$questions = array();
		foreach (array_keys($relations = $this->getChildren()) as $key)
			$questions[] = $relations[$key]->getChild();
		return $questions;
	}
	
	function createDiagnosticAttempt() {
		$attempt = SyndNodeLib::factory('attempt');
		$attempt->setParent($this);
		foreach (array_keys($questions = $this->getQuestions()) as $key)
			$attempt->addQuestion($questions[$key]);
		return $attempt;
	}
	
	function getTrainingAttempt() {
		require_once 'core/model/node/attempt.class.inc';
		return new SyndTrainingAttempt();
	}
	
	function _getCollection($request) {
		if (empty($request['collections']))
			return null;

		if (count($request['collections']) <= 1) 
			$collection = SyndLib::getInstance(SyndLib::reset($request['collections']));
		else {
			$collection = SyndType::factory('composite_collection');
			foreach ((array)$request['collections'] as $id)
				$collection->appendCollection(SyndLib::getInstance($id));
		}
	
		return $collection;
	}
	
	function _view_remove(Request $request, Template $page) {
		if (!$this->isPermitted('write'))
			throw new ForbiddenException();
		if (null == ($collection = $this->_getCollection($request)))
			return null;

		$relations = $collection->getFilteredContents(array('synd_node_relation'));
		foreach (array_keys($relations) as $key) {
			$parent = $relations[$key]->getParent();
			if ($parent->nodeId != $this->nodeId)
				throw new ForbiddenException();
		}

		if (!empty($request['confirm']))
			SyndLib::invoke($relations, 'delete');
		else {
			return array('content' => $page->fetchnode($this, 'confirm_remove_questions.tpl', 
				array('collection' => $collection)));
		}
	}
}
