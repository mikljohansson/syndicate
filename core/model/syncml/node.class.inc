<?php
require_once 'core/model/SyndXMLNode.class.inc';

/**
 * SyncML DOM node
 *
 * Methods employe null pattern meaning they always return an 
 * object satisfying the synd_syncml_node interface. Used the
 * isNull() method to check for null objects.
 *
 * @access		public
 * @package		synd.core.model.syncml
 */
class synd_syncml_node extends SyndXMLNode {
	function synd_syncml_node($name, $content = null) {
		parent::SyndXMLNode($name, $content);
	}
	
	function nullInstance() {
		return synd_syncml_null::instance();
	}

	/**
	 * Creates an instance of a node
	 *
	 * Autoloads the class definition if one exists for the given
	 * tagname.
	 *
	 * @param	string	Tagname of new node
	 * @param	string	Text/CDATA content
	 * @return	synd_syncml_node
	 */
	static function factory($name, $content = null) {
		if (preg_match('/^\w+$/', $name) && file_exists(dirname(__FILE__).'/'.strtolower($name).'.class.inc'))
			include_once('core/model/syncml/'.strtolower($name).'.class.inc');
		if (class_exists($class = 'synd_syncml_'.strtolower($name)))
			return new $class($name, $content);
		return new synd_syncml_node($name, $content);
	}
	
	/**
	 * Used by messages to get responses from their parts
	 * @param	synd_syncml_message	The originating message from client
	 * @param	synd_syncml_message	The response message being built
	 * @return	synd_syncml_node
	 */
	function getResponse($message, $response) {
		return synd_syncml_null::instance();
	}

	/**
	 * Used by messages to get responses from registered senders
	 * @param	synd_syncml_message	The originating message from client
	 * @param	synd_syncml_message	The response message being built
	 * @return	synd_syncml_node
	 */
	function getPackageResponse($message, $response) {
		return synd_syncml_null::instance();
	}

	/**
	 * Used by the Sync element to get response from their commands
	 * @param	synd_syncml_message	The originating message from client
	 * @param	synd_syncml_message	The response message being built
	 * @param	ISyncableCollection	The collection to sync against
	 * @return	synd_syncml_node
	 */
	function getSyncronizeResponse($message, $response, $collection) {
		return synd_syncml_null::instance();
	}

	/**
	 * Used by the Status element to signal their commands
	 * @param	synd_syncml_message	The originating message from client
	 * @param	synd_syncml_message	The response message being built
	 * @param	synd_syncml_status	The status response
	 * @return	synd_syncml_node
	 */
	function getStatusResponse($message, $response, $status) {
		return synd_syncml_null::instance();
	}

	function getTemplateResponse($message) {
		return synd_syncml_null::instance();
	}
	
	/**
	 * Sorting precedence 
	 *
	 * Used by message before calling getResponse() on elements in 
	 * the body such as commands or status. Higher gets run first.
	 *
	 * @return	integer
	 */
	function getPrecedence() {
		return 0;
	}
}

class synd_syncml_null extends synd_syncml_node {
	function synd_syncml_null() {
		parent::synd_syncml_node(null);
	}
	
	/**
	 */
	static function instance() {
		return new synd_syncml_null();
	}
	
	function isNull() {
		return true;
	}
	
	function copy($name = null) {
		return $this;
	}
	
	function appendChild($child) {
		return $child;
	}
	
	function setContent($content) {}
	function setAttribute($name, $value) {}
}
