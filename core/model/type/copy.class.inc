<?php 
require_once 'core/model/type/ole.class.inc';

class synd_type_copy extends synd_type_ole {
	function isValidTarget($target) {
		if (!is_object($target))
			return false;
		foreach (array_keys($children = $this->getInstances()) as $key) {
			if (!$target->isPermitted('append', $children[$key]))
				return false;
		}
		return true;
	}

	function _view_apply(Request $request, Template $page) {
		$children = $this->getInstances();
		$target = null;
		$before = null;
		
		foreach (array_keys($children) as $key) {
			if (!$children[$key]->isPermitted('read'))
				throw new ForbiddenException();
		}
		
		if (isset($request[2]))
			$target = SyndLib::getInstance($request[2]);
		if (null != ($collection = $this->_getCollection($request)) && count($contents = $collection->getContents())) {
			if (empty($request[3]) && null !== ($parent = SyndLib::end($contents)->getParent()) && $this->isValidTarget($parent))
				$target = $parent;
			$before = SyndLib::end($contents);
		}
		
		if (!$this->isValidTarget($target))
			throw new InvalidArgumentException();

		foreach (array_keys($children) as $key) {
			if (false !== ($clone = $target->insertBefore($children[$key]->copy(), $before)))
				$clone->save();
			assert('false !== $clone');
		}
			
		$this->cancel();
	}
}
