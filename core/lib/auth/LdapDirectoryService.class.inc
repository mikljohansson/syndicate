<?php
require_once 'core/lib/auth/DirectoryService.class.inc';

class LdapDirectoryService implements DirectoryService {
	function getOrganisation($id);
	function getRole($id);
	function getUser($id);

	function findOrganisations($query);
	function findRoles($query);
	function findUsers($query);

	function authenticate(User $user, $password) {
		return false;
	}
}
