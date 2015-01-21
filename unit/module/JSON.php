<?
require_once 'RpcTestcase.class.inc';

class _modules_JSON extends RpcTestcase {
	function getActivator() {
		return new RpcModuleActivator(Module::getInstance('rpc'), 'json');
	}
}
