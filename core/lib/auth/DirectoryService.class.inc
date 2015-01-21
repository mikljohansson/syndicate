<?php

/**
 * A directory service handles user lookup and authentication
 * @access		public
 * @package		synd.core.model
 */
interface DirectoryService {
	function getOrganisation($id);
	function getRole($id);
	function getUser($id);

	function findOrganisations($query);
	function findRoles($query);
	function findUsers($query);

	function authenticate(User $user, $password);
}

class ForwardingDirectoryService extends DirectoryService {
	protected $_next = null;
	
	function __construct($next) {
		$this->_next = $next;
	}

	function getOrganisation($id)						{return $this->_next->getOrganisation($id);}
	function getRole($id)								{return $this->_next->getRole($id);}
	function getUser($id)								{return $this->_next->getUser($id);}

	function findOrganisations($query)					{return $this->_next->findOrganisations($query);}
	function findRoles($query)							{return $this->_next->findRoles($query);}
	function findUsers($query)							{return $this->_next->findUsers($query);}

	function authenticate(User $user, $password)		{return $this->_next->authenticate($user, $password);}
}
