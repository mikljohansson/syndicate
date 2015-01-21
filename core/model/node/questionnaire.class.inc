<?php
require_once 'core/model/node/page.class.inc';
require_once 'core/model/node/case.class.inc';

class synd_node_questionnaire extends synd_node_page {
	/**
	 * Cached ids of children
	 */
	var $_children = null;

	function getHandler() {
		$parent = $this->getParent();
		return $parent->getHandler();
	}

	function getImplodedTree() {
		return array();
	}

	function getPage() {
		return $this->getParent();
	}

	function getPageNumber() {
		$parent = $this->getParent();
		if (!($parent instanceof synd_node_page)) 
			return null;
		
		$i = 0;
		foreach (array_keys($chapters = $parent->getChapters()) as $key) {
			if ($chapters[$key]->nodeId == $this->nodeId) break;
			if (($chapters[$key] instanceof synd_node_questionnaire)) $i++;
		}
				
		return trim($parent->getPageNumber().'.E'.($i+1),'.');
	}
	
	function hasPermission($user, $perm, $arg = null) {
		if ('append' == $perm && $this->hasPermission($user, 'write'))
			return ($arg instanceof synd_node_questionnaire) || ($arg instanceof synd_node_question);
		return parent::hasPermission($user, $perm, $arg);
	}

	function getChildren() {
		if (null === $this->_children) {
			$sql = "
				SELECT n.node_id, p.info_weight FROM synd_node n, synd_node_page p
				WHERE 
					n.node_id = p.node_id AND
					n.parent_node_id = ".$this->_db->quote($this->nodeId)."
				UNION
				SELECT q.node_id, q.info_weight FROM synd_question q
				WHERE
					q.parent_node_id = ".$this->_db->quote($this->nodeId)."
				ORDER BY info_weight";
			$this->_children = (array)$this->_db->getCol($sql);
		}
		return $this->_storage->getInstances($this->_children);
	}
	
	function getQuestions() {
		$sql = "
			SELECT q.node_id, q.info_weight FROM synd_question q
			WHERE q.parent_node_id = ".$this->_db->quote($this->nodeId);
		$questions = $this->_storage->getInstances($this->_db->getCol($sql));
		
		foreach ($this->getChildren() as $key => $value) {
			if ($value instanceof synd_node_questionnaire)
				$questions = array_merge($questions, $value->getQuestions());
		}
		
		return $questions;
	}

	function getDiagnosticQuestions() {
		$sql = "
			SELECT q.node_id, q.info_weight FROM synd_question q
			WHERE 
				q.parent_node_id = ".$this->_db->quote($this->nodeId)." AND
				q.flag_diagnostic = 1
			ORDER BY q.info_weight";
		return array_merge(parent::getDiagnosticQuestions(), $this->_storage->getInstances($this->_db->getCol($sql)));
	}
	
	function getQuestionnaires() {
		$questionnaires = parent::getQuestionnaires();
		$questionnaires[$this->nodeId] = $this;
		return $questionnaires;
	}

	function getProgressQuestions($limit = null) {
		$sql = "
			SELECT q.node_id, q.info_weight FROM synd_question q
			WHERE 
				q.parent_node_id = ".$this->_db->quote($this->nodeId)." AND
				q.flag_progress = 1
			ORDER BY q.info_weight";
		return $this->_storage->getInstances($this->_db->getCol($sql,0,0,$limit));
	}
	
	function getNext($key = null) {
		$parent = $this->getParent();
		if (!$parent->isNull()) {
			$result = $parent->getNext($this->nodeId);
			return $result;
		}
		$result = null;
		return $result;
	}
	
	function getLast() {
		return $this;
	}
	
	function getTrainingAttempt() {
		require_once 'core/model/node/attempt.class.inc';
		return new SyndTrainingAttempt();
	}
	
	function addStatisticsFilter($question, $option) {
		$parent = $this->getParent();
		$parent->addStatisticsFilter($question, $option);
	}
	
	function delStatisticsFilter($question, $option) {
		$parent = $this->getParent();
		$parent->delStatisticsFilter($question, $option);
	}
	
	function getStatisticsFilters() {
		$parent = $this->getParent();
		return $parent->getStatisticsFilters();
	}
	
	function _node_onchange_child() {
		$this->_children = null;
		parent::_node_onchange_child();
	}
}
