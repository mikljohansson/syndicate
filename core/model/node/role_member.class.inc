<?php
require_once 'core/model/node/case.class.inc';

/**
 * Callback to allow subjects to determine if a user has authorization
 */
class synd_node_role_member extends synd_node_case {
	function toString() {
		return SyndLib::translate('Members');
	}
	
	function hasAuthorization(Instance $user, Instance $subject) {
		return $subject->hasAuthorization($user, $this) || parent::hasAuthorization($user, $subject);
	}
}
