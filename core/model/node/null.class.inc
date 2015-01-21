<?php
require_once 'core/model/node/case.class.inc';

class synd_node_null extends synd_node_case {
	function isNull()		{return true;}

	function __construct($id, $defer)	{
		parent::__construct($id, $defer);
		$this->nodeId = null;
	}
	
	function __sleep() 		{return array();}

	function objectId()		{return null;}
	function id()   {return 'node.'.$this->nodeId();}
	function toString()		{return null;}

	function hasPermission($user, $perm, $arg = null) {
		if ('remove' == $perm)
			return true;
		return false;
	}

	function getIndexDocument($index) {
		return $index->createComposite(array(
			$index->createFragment('null'),
			parent::getIndexDocument($index)));
	}
}
