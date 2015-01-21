<?php
require_once 'core/model/syncml/node.class.inc';

class synd_syncml_meta extends synd_syncml_node {
	function synd_syncml_meta($name = 'Meta') {
		parent::synd_syncml_node($name);
	}
	
	function appendChild($child) {
		if (null == $child->getAttribute('xmlns'))
			$child->setAttribute('xmlns', 'syncml:metinf');
		return parent::appendChild($child);
	}
}
