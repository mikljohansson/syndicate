<?php
require_once 'core/model/syncml/location.class.inc';

class synd_syncml_target extends synd_syncml_location {
	function synd_syncml_target($name = 'Target', $locURI = null, $locName = null) {
		parent::synd_syncml_location($name, $locURI, $locName);
	}

	function getTemplateResponse($message) {
		$locuri = $this->getElement('LocURI');
		return $locuri->clone('TargetRef');
	}
}
