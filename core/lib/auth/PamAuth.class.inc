<?php
require_once 'core/lib/auth/DirectoryService.class.inc';

class PamAuth extends ForwardingDirectoryService {
	function authenticate(User $user, $password) {
		if (strlen($password) && pam_auth($user->getLogin(), $password))
			return true;
		return parent::authenticate($user, $password);
	}
}
