<?php
require_once 'core/model/node/relation.class.inc';

class synd_node_repaired extends synd_node_relation {
	function getHandler() {
		$issue = $this->getIssue();
		return $issue->getHandler();
	}
	
	function setItem($item) {
		parent::setParent($item);
	}

	function getItem() {
		$item = parent::getParent();
		if (($item instanceof synd_node_used))
			return $item->getItem();
		return $item;
	}

	function setIssue($issue) {
		parent::setChild($issue);
	}

	function getIssue() {
		return parent::getChild();
	}
}
