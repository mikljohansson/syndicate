<?php
require_once 'core/model/node/case.class.inc';

class synd_node_role_authenticated extends synd_node_case {
	function toString() {
		return SyndLib::translate('Logged in users');
	}
	
	function hasAuthorization(Instance $user, Instance $subject) {
		return !$user->isNull() || parent::hasAuthorization($user, $subject);
	}
}
