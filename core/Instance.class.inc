<?php

/**
 * Marshal by reference object
 *
 * A registered instance can be loaded by giving its id to the 
 * 'instance' hook, which is used for example by the request
 * handles and rpc modules to load instances.
 *
 * @access		public
 * @package		synd.core
 */
interface Instance {
	function id();
}
