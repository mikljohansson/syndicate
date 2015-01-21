<?php
require_once 'core/model/node/relation.class.inc';

/**
 * Relation between a lease termination issue and an individual leasing.
 */
class synd_node_terminated extends synd_node_relation {
	function getItem() {
		$item = $this->getChild();
		if (($item instanceof synd_node_used))
			return $item->getItem();
		return $item;
	}
	
	function setItem($relation) {
		$this->setChild($relation);
	}
	
	function getRelation() {
		return $this->getChild();
	}

	function setRelation($relation) {
		$this->setChild($relation);
	}
	
	function getIssue() {
		return $this->getParent();
	}
	
	function setIssue($issue) {
		$this->setParent($issue);
	}
}
