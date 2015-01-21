<?php
require_once 'core/module/rpc.class.inc';

/**
 * @deprecated	Use synd_module_rpc with a custom formatter
 */
class synd_module_soap extends synd_module_rpc {
	function request(Request $request) {
		$request->setContext(array_merge(array($this->classId()), $request->getContext()));
		return parent::request($request);
	}
}
