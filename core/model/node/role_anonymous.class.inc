<?php
require_once 'core/model/node/case.class.inc';

class synd_node_role_anonymous extends synd_node_case {
	function toString() {
		return SyndLib::translate('Non logged in users');
	}
	
	function isNull() {
		return true;
	}
	
	function hasAuthorization(Instance $user, Instance $subject) {
		return $user->isNull() || parent::hasAuthorization($user, $subject);
	}
}
