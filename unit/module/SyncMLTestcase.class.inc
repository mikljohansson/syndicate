<?php
require_once 'PHPUnit2/Framework/TestCase.php';
require_once 'core/model/ISyncable.class.inc';
require_once 'core/lib/calendar/VCalendar.class.inc';

/**
 * @package		synd.unit
 */
abstract class SyncMLTestcase extends PHPUnit2_Framework_TestCase {
	function setUp() {
		_SyncableUnittestCollection::instance(true);
	}

	function tearDown() {
		SyndLib::runHook('variable_set', 'syncml', $_SERVER['SERVER_ADDR'].'.1', null);
	}
}

/**
 * @access	private
 */
class _SyncableUnittestCollection extends ISyncableCollection {
	var $_contents = array();	
	var $_replaced = array();
	var $_appended = array();
	var $_removed = array();
	var $_id = 1;
	
	function instance($new = false) {
		static $instance = null;
		if (null === $instance || $new)
			$instance = new _SyncableUnittestCollection();
		return $instance;
	}
	
	function getInstance($id) {
		global $_syncable_unittest_instances;
		if (!isset($_syncable_unittest_instances[$id]))
			$_syncable_unittest_instances[$id] = null;
		return $_syncable_unittest_instances[$id];
	}

	function isNull() {
		return false;
	}
	
	function getName() {
		return '_unit_test';
	}
	
	function getCalendarCapabilities() {
		return array(
			'BEGIN' => array('VTODO'),
			'UID' => true,
			'URL' => true,
			'DTSTAMP' => true,
			'CREATED' => true,
			'LAST-MOD' => true,
			'DUE' => true,
			'DTSTART' => true,
			'COMPLETED' => true,
			'PRIORITY' => true,
			'CATEGORIES' => true,
			'STATUS' => true,
			'SUMMARY' => true,
			'DESCRIPTION' => true,
			'COMMENT' => true,
			'END' => array('VTODO'),
			);
	}
	
	function getContents($last = null, $next = null) {
		return $this->_contents;
	}
	
	function append($memento) {
		global $_syncable_unittest_instances;
		while (isset($_syncable_unittest_instances['_syncml._unit_test'.$this->_id]))
			$this->_id++;

		$this->_appended[] = $memento;
		$instance = new _SyncableUnittestInstance('_syncml._unit_test'.$this->_id, $memento);
		$this->_contents[$instance->id()] = $instance;
		return $instance;
	}

	function remove($instance) {
		$this->_removed[] = $instance;
		if (isset($this->_contents[$instance->id()]))
			unset($this->_contents[$instance->id()]);
		return $instance;
	}
}

/**
 * @access	private
 */
class _SyncableUnittestInstance {
	var $_id = null;
	var $_memento = null;
	var $_nonAnonymous = false;
	
	function _SyncableUnittestInstance($id, $memento = null, $nonAnonymous = false) {
		global $_syncable_unittest_instances;
		$_syncable_unittest_instances[$id] = $this;
		$this->_id = $id;
		$this->_memento = $memento;
		$this->_nonAnonymous = $nonAnonymous;
	}
	
	function id() {
		return $this->_id;
	}
	
	function getMemento() {
		if (null != $this->_memento)
			return $this->_memento;
		return new VCalendarObject();
	}
	
	function setMemento($memento) {
		$this->_memento = $memento;
		return true;
	}
	
	function hasPermission($user, $perm) {
		return !$this->_nonAnonymous || !$user->isNull();
	}
}

/**
 * @access	private
 */
function _callback_syncronize_unittest(&$result, $name, $user, $message) {
	if ('_unit_test' == strtolower(trim($name,'./'))) {
		$result[] = _SyncableUnittestCollection::instance();
		return true;
	}
	if ('_unit_test_unauthorized' == strtolower(trim($name,'./'))) {
		if ($user[0]->isNull()) {
			$result[] = new SyncableUnauthorizedCollection();
			return true;
		}
		$result[] = _SyncableUnittestCollection::instance();
		return true;
	}
}

SyndLib::attachHook('syncronize', '_callback_syncronize_unittest');
