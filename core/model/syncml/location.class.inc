<?php
require_once 'core/model/syncml/node.class.inc';

class synd_syncml_location extends synd_syncml_node {
	function synd_syncml_location($name, $locURI = null, $locName = null) {
		parent::synd_syncml_node($name);
		if (null != $locURI) 
			$this->appendChild(new synd_syncml_node('LocURI', $locURI));
		if (null != $locName) 
			$this->appendChild(new synd_syncml_node('LocName', $locName));
	}
}
