<?php
require_once 'core/Instance.class.inc';

class synd_type_ole extends SyndType {
	/**
	 * Ids of contained instances
	 */
	var $_ids = array();
	
	/**
	 * Whether this action has been cancelled
	 */
	var $_cancel = false;
	
	function __construct() {
		parent::__construct();
		$GLOBALS['_synd_ole'][$this->id()] = $this;
	}

	function __wakeup() {
		$GLOBALS['_synd_ole'][$this->id()] = $this;
	}

	static function __instance($id) {
		if (isset($GLOBALS['_synd_ole'][$id]))
			return $GLOBALS['_synd_ole'][$id];
		return parent::__instance($id);
	}

	function _getCollection($request) {
		if (empty($request['collections']))
			return null;
		
		$collections = (array)$request['collections'];
		if (count($collections) <= 1) 
			$collection = SyndLib::getInstance(reset($collections));
		else {
			$collection = SyndType::factory('composite_collection');
			foreach ($collections as $id)
				$collection->appendCollection(SyndLib::getInstance($id));
		}
	
		return $collection;
	}
	
	/**
	 * Appends an instances to this action
	 * @param	Instance	The instances to append
	 * @return	Instance	Returns the appended instances
	 */
	function appendChild(Instance $child) {
		if (false === array_search($child->id(), $this->_ids))
			$this->_ids[] = $child->id();
		return $child;
	}
	
	/**
	 * Returns the contained instances
	 * @return	array	Array of Instance
	 */
	function getInstances() {
		return SyndLib::getInstances($this->_ids);
	}
	
	function isValidTarget($node) {
		return false;
	}
	
	function isCancelled() {
		return $this->_cancel;
	}
	
	function delete() {
		parent::delete();
		unset($GLOBALS['_synd_ole'][$this->id()]);
	}
	
	function cancel() {
		$this->_cancel = true;
	}
	
	function _view_cancel(Request $request, Template $page) {
		$this->cancel();
	}

	/**
	 * Applies this action to a target
	 * @param	string	Optional instance id of target
	 * @param	bool	Force apply to specified target
	 */
	function _view_apply(Request $request, Template $page) {}
}
