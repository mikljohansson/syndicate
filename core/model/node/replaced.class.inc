<?php
require_once 'core/model/node/repaired.class.inc';

class synd_node_replaced extends synd_node_repaired {
	static function getEntityQuery(DomainStorage $storage) {
		$query = parent::getEntityQuery($storage);
		$query->join('synd_issue_replaced');
		return $query;
	}
	
	function getReplacement() {
		if (null != ($replacement = SyndNodeLib::getInstance($this->data['REPLACEMENT_NODE_ID'])))
			return $replacement;
		return SyndNodeLib::getInstance('null.null');
	}
	
	function setReplacement($replacement) {
		$this->data['REPLACEMENT_NODE_ID'] = $replacement->nodeId;
		$this->_node_onchange_internal();
	}
}
