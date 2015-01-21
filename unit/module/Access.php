<?php
require_once 'PHPUnit2/Framework/TestCase.php';
require_once 'core/model/DomainObject.class.inc';

class _modules_Access extends SyndNodeTestCase {
	var $_stubs = array();
	
	function setUp() {
		global $synd_maindb;
		if (null === Module::getInstance('access'))
			Module::loadModule('access');
		$synd_maindb->begin();
	}
	
	function tearDown() {
		global $synd_maindb;
		$synd_maindb->rollback();
	}
	
	function testPermitted() {
		$access = Module::getInstance('access');

		$parent = new _AccessSubject('parent');
		$subject = new _AccessSubject('subject', $parent);
		
		$user = new _AccessUser('user');
		$role = new _AccessRole('role');
		$role->addMember($user);
		
		// Check basic permissions
		$this->assertFalse($subject->hasPermission($user, 'perm1'));
		$access->grantPermission($subject, $user, 'perm1');
		$this->assertTrue($subject->hasPermission($user, 'perm1'));

		// Check bubble roles
		$this->assertFalse($subject->hasPermission($user, 'perm2'));
		$access->grantPermission($subject, $role, 'perm2');
		$this->assertTrue($subject->hasPermission($user, 'perm2'));

		// Check bubble permission
		$this->assertFalse($subject->hasPermission($user, 'perm3'));
		$access->grantPermission($parent, $user, 'perm3');
		$this->assertTrue($subject->hasPermission($user, 'perm3'));

		// Check bubble both roles and permission
		$this->assertFalse($subject->hasPermission($user, 'perm4'));
		$access->grantPermission($parent, $role, 'perm4');
		$this->assertTrue($subject->hasPermission($user, 'perm4'));

		// Check statically defined roles
		$this->assertFalse($subject->hasPermission($user, 'perm5'));
		$access->grantPermission($subject, SyndNodeLib::getInstance('role_authenticated.Authenticated'), 'perm5');
		$this->assertTrue($subject->hasPermission($user, 'perm5'));

		$this->assertFalse($subject->hasPermission(SyndNodeLib::getInstance('user_null.null'), 'perm6'));
		$access->grantPermission($subject, SyndNodeLib::getInstance('role_anonymous.Anonymous'), 'perm6');
		$this->assertTrue($subject->hasPermission(SyndNodeLib::getInstance('user_null.null'), 'perm6'));
	}
	
	function testAuthorization() {
		$null = SyndNodeLib::getInstance('null.null');
		$this->assertFalse($null->hasAuthorization($null, $null));
		
		$user = SyndNodeLib::getInstance('user_null.null');
		$this->assertFalse($user->hasAuthorization($user, $user));
		$this->assertFalse($null->hasAuthorization($user, $null));
		
		$case = SyndNodeLib::getInstance('case._unit_test');
		$auth = SyndNodeLib::getInstance('role_authenticated.Authenticated');
		$anon = SyndNodeLib::getInstance('role_anonymous.Anonymous');
		$this->assertFalse($auth->hasAuthorization($anon, $case));
	}
	
	function testRoleMembership() {
		$access = Module::getInstance('access');

		$subject = new _AccessSubject('subject');

		$user = new _AccessUser('user');
		$role = new _AccessRole('role');
		$role->addMember($user);
		
		$this->assertFalse($subject->hasPermission($user, 'perm1'));
		$access->grantPermission($subject, $role, 'perm1');
		$this->assertTrue($subject->hasPermission($user, 'perm1'));
	}
	
//	function testRoleSubjectCallback() {
//		$access = Module::getInstance('access');
//	
//		$user = new _AccessUser('user');
//
//		$parent = new _AccessSubject('parent');
//		$parent->addMember($user);
//
//		$subject = new _AccessSubject('subject', $parent);
//		$role = new _AccessMemberRole('role_member');
//		
//		$this->assertFalse($subject->hasPermission($user, 'perm1'));
//		$access->grantPermission($parent, $role, 'perm1');
//		$this->assertTrue($parent->hasPermission($role, 'perm1'));
//		$this->assertTrue($parent->hasPermission($user, 'perm1'));
//		$this->assertFalse($subject->hasPermission($user, 'perm1'));
//
//		$subject->addMember($user);
//		$this->assertTrue($subject->hasPermission($user, 'perm1'));
//	}
}

/**
 * @access	private
 */
class _AccessStub extends AbstractDomainObject {
	public $nodeId = null;
	private static $_instances = array();
	
	function __construct($parent = null, $id = null) {
		$this->nodeId = null != $id ? 'stub.'.$id.'_'.md5(uniqid('')) : 'stub.'.md5(uniqid(''));
		self::$_instances[$this->nodeId] = $this;
	}
	
	function nodeId() {
		return $this->nodeId;
	}
	
	function id() {
		return $this->nodeId;
	}
	
	static function getDefaultStorage() {
		return new NullStorage();
	}
	
	static function _callback_instance(&$result, $id) {
		if (isset(self::$_instances[$id])) {
			$result = self::$_instances[$id];
			return true;
		}
	}
}

SyndLib::attachHook('instance', array('_AccessStub', '_callback_instance'));

/**
 * @access	private
 */
class _AccessSubject extends _AccessStub {
	var $_parent = null;
	var $_members = array();

	function __construct($id = null, $parent = null) {
		parent::__construct($id);
		$this->_parent = $parent;
	}

	function getParent() {
		if (null !== $this->_parent)
			return $this->_parent;
		return parent::getParent();
	}

	function addMember($user) {
		$this->_members[] = $user->id();
	}

	function hasAuthorization(Instance $user, Instance $subject) {
		if (in_array($user->id(), $this->_members))
			return true;
		return parent::hasAuthorization($user, $subject);
	}
}

/**
 * @access	private
 */
class _AccessUser extends _AccessStub {}

/**
 * @access	private
 */
class _AccessRole extends _AccessSubject {}

/**
 * @access	private
 */
class _AccessMemberRole extends _AccessStub {
	function __construct($id = null) {
		parent::__construct(null, $id);
	}

	function hasAuthorization(Instance $user, Instance $subject) {
		return $subject->hasAuthorization($user, $this);
	}	
}
