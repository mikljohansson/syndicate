<?php
require_once 'core/model/node/case.class.inc';

class synd_node_guestbook extends synd_node_case {
	function hasPermission($user, $perm, $arg = null) {
		if ('append' == $perm && ($arg instanceof synd_node_comment))
			return true;
		return parent::hasPermission($user, $perm, $arg);
	}
}
