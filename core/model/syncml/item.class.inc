<?php
require_once 'core/model/syncml/node.class.inc';

class synd_syncml_item extends synd_syncml_node {
	function synd_syncml_item($name = 'Item') {
		parent::synd_syncml_node($name);
	}

	function getTemplateResponse($message) {
		$node = new synd_syncml_item();

		$target = $this->getElement('Target');
		$node->appendChild($target->clone('Source'));

		return $node;
	}
}
