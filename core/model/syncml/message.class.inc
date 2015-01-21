<?php
require_once 'core/model/syncml/node.class.inc';

/**
 * SyncML message
 *
 * Contains the message XML document. Allows for parsing a message 
 * into the corresponding DOM.
 *
 * @access		public
 * @package		synd.core.model.syncml
 */
class synd_syncml_message {
	var $_id = 1;
	var $_root = null;
	var $_stack = null;
	var $_isBinary = false;
	
	var $_decoder = array('/usr/bin/wbxml2xml', '-v 1.2 -m 2 -o - -');
	var $_encoder = array('/usr/bin/xml2wbxml', '-n -o - -');
	
	/**
	 * @param	string	XML or binary WBXML content
	 * @param	string	Content type of $content
	 */
	function synd_syncml_message($content = null, $type = null) {
		if (null != $content) {
			if ('application/vnd.syncml+wbxml' != strtolower(reset(explode(';',$type))) && '<' == substr($content,0,1)) 
				$this->_parse($content);
			else if (null != ($xml = $this->_decode($content))) {
				$this->_isBinary = true;
				$this->_parse($xml);
			}
		}
	}

	/**
	 * Returns the Content-Type of this message
	 * @return	string
	 */
	function getContentType() {
		return $this->_isBinary ? 
			'application/vnd.syncml+wbxml; charset="utf-8"' : 
			'application/vnd.syncml+xml; charset="utf-8"';
	}

	/**
	 * @access	private
	 */
	function _decode($wbxml) {
		if (SyndDLE::functionExistsDL('wbxml_decode', 'wbxml'))
			return wbxml_decode($wbxml);
		if (is_executable($this->_decoder[0]))
			return SyndLib::execute(implode(' ', $this->_decoder), $wbxml, true, 1024000);
		trigger_error('No WBXML decoder available.', E_USER_WARNING);
	}
	
	/**
	 * @access	private
	 */
	function _encode($xml) {
		if (SyndDLE::functionExistsDL('wbxml_encode', 'wbxml'))
			return wbxml_encode($xml);
		if (is_executable($this->_encoder[0]))
			return SyndLib::execute(implode(' ', $this->_encoder), $xml, true, 1024000);
		if ((@include_once('Horde/WBXML/Encoder.php'))) {
			error_reporting(E_ALL ^ E_NOTICE ^ E_USER_NOTICE);
			$encoder = new XML_WBXML_Encoder();
			return $encoder->encode($xml);
		}
		trigger_error('No WBXML encoder available.', E_USER_WARNING);
	}
	
	/**
	 * Append the root node ('SyncML')
	 * @see		_parse()
	 * @param	synd_syncml_node
	 * @return	synd_syncml_node
	 */
	function appendChild($child) {
		if (null == $child->getAttribute('xmlns'))
			$child->setAttribute('xmlns', 'syncml:SYNCML1.1');
		$this->_root = $child;
		return $child;
	}
	
	/**
	 * @see		_parse()
	 * @access	private
	 */
	function setContent($content) {}

	/**
	 * @see		_parse()
	 * @access	private
	 */
	function setAttribute($name, $value) {}
	
	/**
	 * Returns the root node ('SyncML')
	 * @return	synd_syncml_node
	 */
	function getDocument() {
		if (null !== $this->_root)
			return $this->_root;
		return synd_syncml_null::instance();
	}
	
	/**
	 * Renders this message into a string
	 *
	 * Binary messages will be encoded into wbxml 
	 *
	 * @see		getContentType()
	 * @return	string
	 */
	function toString() {
		$xml = "<?xml version=\"1.0\"?>\r\n";
		$xml .= "<!DOCTYPE SyncML PUBLIC \"-//SYNCML//DTD SyncML 1.1//EN\" \"http://www.syncml.org/docs/syncml_represent_v11_20020213.dtd\">\r\n";
		$xml .= $this->_root->toString();
		return $this->_isBinary ? $this->_encode($xml) : $xml;
	}
	
	/**
	 * Checks if this message is oversize with regards to MaxMsgSize
	 * @return	bool
	 */
	function isOversized() {
		$session = $this->getSession();
		if (null == $session->getMaxMsgSize())
			return false;
		if ($session->getMaxMsgSize() > 5000)
			return strlen($this->toString()) > $session->getMaxMsgSize()-250;
		return strlen($this->toString()) > $session->getMaxMsgSize();
	}
	
	/**
	 * Checks out a new command id serial number
	 * @return	integer
	 */
	function nextCommandId() {
		return (string)($this->_id++);
	}
	
	/**
	 * Returns the message id ('SyncML','SyndHdr','MsgID')
	 * @return	integer
	 */
	function getMessageId() {
		$document = $this->getDocument();
		$msgid = $document->selectSingleNode('SyncHdr', 'MsgID');
		return $msgid->getContent();
	}
	
	/**
	 * Returns all command sorted by precedence
	 * @return	array
	 */
	function getCommands() {
		$document = $this->getDocument();
		$body = $document->getElement('SyncBody');
		return array_reverse(SyndLib::sort(SyndLib::array_merge_assoc(
			array($document->getElement('SyncHdr')), $body->getChildren()), 'getPrecedence'));
	}
	
	/**
	 * Builds a response message
	 * @uses	synd_syncml_node::getResponse()
	 * @return	synd_syncml_message
	 */
	function getResponse() {
		$message = new synd_syncml_message();
		$message->_isBinary = $this->_isBinary;
		$response = $message->appendChild(synd_syncml_node::factory('SyncML'));
		
		$document = $this->getDocument();
		$header = $document->getElement('SyncHdr');
		$body = $document->getElement('SyncBody');
		
		$response->appendChild($header->getTemplateResponse($this));
		$responseBody = $response->appendChild(synd_syncml_node::factory('SyncBody'));
		$session = $this->getSession();
		
		// Store MaxMsgSize in session if supplied
		$maxMsgSize = $header->selectSingleNode('Meta', 'MaxMsgSize');
		if (!$maxMsgSize->isNull())
			$session->setMaxMsgSize($maxMsgSize->getContent());
		
		// Run the response hook (SyncHdr, Status, Sync and other command elements)
		foreach (array_keys($commands = $this->getCommands()) as $key)
			$responseBody->appendChild($commands[$key]->getResponse($this, $message));

		// Run the response hook on registered commands (Alert)
		$commands = $session->getPackageCommands();
		$session->clearPackageCommands();
		
		foreach (array_keys($commands) as $key)
			$responseBody->appendChild($commands[$key]->getPackageResponse($this, $message));
		
		// If whole package is sent, append Final element
		if (!count($session->getPackageCommands()))
			$responseBody->appendChild(synd_syncml_node::factory('Final'));
		
		return $message;
	}
	
	/**
	 * Runs the 'syncronize' hook
	 * @todo	Composite collection pattern should be used if multiple collections received
	 * @param	string	Collection name or path (./tasks)
	 * @return	ISyncableCollection
	 */
	function getCollection($name) {
		$session = $this->getSession();
		$user = $session->getAuthenticatedUser();
		$collections = SyndLib::runHook('syncronize', $name, array($user), array($this));
		if (empty($collections))
			return new SyncableNullCollection();
		return $collections[key($collections)];
	}
	
	/**
	 * Returns the current session
	 * @return	SyncMLSession
	 */
	function getSession() {
		if (null == ($session = SyndLib::runHook('variable_get', 'syncml', $this->getSessionId()))) {
			$session = new SyncMLSession($this->getSessionId());
			SyndLib::runHook('variable_set', 'syncml', $this->getSessionId(), $session, 3600*24);
		}
		return $session;
	}
	
	/**
	 * Returns the sessionid (based on 'SyncML','SyndHdr','SessionID')
	 * @return	string
	 */
	function getSessionId() {
		$document = $this->getDocument();
		$header = $document->getElement('SyncHdr');
		$sid = $header->getElement('SessionID');
		return $_SERVER['REMOTE_ADDR'].'.'.$sid->getContent();
	}
	
	/**
	 * Returns the persistent session
	 * @return	SyncMLPersistentSession
	 */
	function getPersistentSession() {
		$document = $this->getDocument();
		$header = $document->getElement('SyncHdr');
		$source = $header->getElement('Source');
		$locuri = $source->getElement('LocURI');
		$id = 'syncml.session.'.md5($locuri->getContent());
		
		if (null !== ($instance = SyndLib::getVar($id)))
			return $instance;
		return new SyncMLPersistentSession($id);
	}
	
	/**
	 * Frees resources (sessions) associated with this message
	 */
	function delete() {
		SyndLib::runHook('variable_set', 'syncml', $this->getSessionId(), $session, null);
		$persistent = $this->getPersistentSession();
		$persistent->delete();
	}
	
	/**
	 * Parses a SyncML message into this message
	 * @access	private
	 * @param	string	XML to parse
	 */
	function _parse($xml) {
		$parser = xml_parser_create('ISO-8859-1');
		xml_parser_set_option($parser, XML_OPTION_CASE_FOLDING, false);
		xml_parser_set_option($parser, XML_OPTION_SKIP_WHITE, true);
		
		xml_set_element_handler($parser, 
			array($this, '_callback_enter'),
			array($this, '_callback_leave'));
		xml_set_character_data_handler($parser, 
			array($this, '_callback_cdata'));

		$this->_stack[] = $this;
		xml_parse($parser, $xml, true);

		$this->_stack = null;
		xml_parser_free($parser);
	}
	
	/**
	 * @access	private
	 */
	function _callback_enter($parser, $name, $attributes) {
		$node = synd_syncml_node::factory($name);
		foreach ($attributes as $key => $value)
			$node->setAttribute($key, $value);

		$current = $this->_stack[count($this->_stack)-1];
		$current->appendChild($node);

		$this->_stack[] = $node;
	}
	
	/**
	 * @access	private
	 */
	function _callback_leave($parser, $name) {
		$current = $this->_stack[count($this->_stack)-1];
		if (null != $current->_content)
			$current->_content = trim($current->_content);
		array_pop($this->_stack);
	}

	/**
	 * @access	private
	 */
	function _callback_cdata($parser, $cdata) {
		$current = $this->_stack[count($this->_stack)-1];
		if ('' != ($content = trim($cdata)) || null != $current->_content)
			$current->_content .= "\n" == $cdata ? "\r\n" : $cdata;
	}
}

/**
 * SyncML session stored on heap
 *
 * @access		public
 * @package		synd.core.model
 */
class SyncMLSession {
	var $_id = null;
	var $_user = null;
	var $_nonce = null;
	var $_maxMsgSize = null;

	var $_nextMessageId = 1;
	var $_commands = array();
	var $_package = array();
	
	function SyncMLSession($id) {
		$this->_id = $id;
	}

	function delete() {}
	
	function getSessionId() {
		return $this->_id;
	}
	
	function nextMessageId() {
		return $this->_nextMessageId++;
	}

	function nextNonce() {
		return $this->_nonce = substr(md5(uniqid('')),0,8);
	}
	
	function getNonce() {
		return $this->_nonce;
	}
	
	function getMaxMsgSize() {
		return $this->_maxMsgSize;
	}
	
	function setMaxMsgSize($size) {
		$this->_maxMsgSize = (int)$size;
	}

	function getAuthenticatedUser() {
		if (null != ($user = SyndNodeLib::getInstance($this->_user)))
			return $user;
		return SyndNodeLib::getInstance('user_null.null');
	}
	
	function setAuthenticatedUser($user) {
		$this->_user = $user->nodeId;
	}

	function getCommand($msgid, $cmdid) {
		if (!isset($this->_commands[$msgid.'.'.$cmdid])) 
			return synd_syncml_null::instance();
		return $this->_commands[$msgid.'.'.$cmdid];
	}
	
	function getCommands() {
		return $this->_commands;
	}
	
	function removeCommand($msgid, $cmdid) {
		if (isset($this->_commands[$msgid.'.'.$cmdid])) 
			unset($this->_commands[$msgid.'.'.$cmdid]);
	}
	
	function setCommand($msgid, $cmdid, $command) {
		$this->_commands[$msgid.'.'.$cmdid] = $command;
	}
	
	function getPackageCommands() {
		return $this->_package;
	}
	
	/**
	 * Register a sender
	 *
	 * Reregistration must be done in the getPackageResponse() hook 
	 * if futher sending must be done, such as when sending multiple
	 * messages in a package
	 *
	 * @see		synd_syncml_node::getPackageResponse()
	 */
	function addPackageCommand($command) {
		$this->_package[] = $command;
	}
	
	function clearPackageCommands() {
		$this->_package = array();
	}
}

/**
 * The persistent session stores sync anchors, GUID-LUID mappings, etc.
 */
class SyncMLPersistentSession {
	var $_id = null;
	var $_devinfo = null;
	var $_anchors = array();
	var $_map = array();
	
	function __construct($id) {
		$this->_id = $id;
	}
	
	function __destruct() {
		SyndLib::setVar($this->_id, $this);
	}
	
	function getDeviceInfo() {
		if (null != $this->_devinfo)
			return $this->_devinfo;
		return synd_syncml_null::instance();
	}
	
	function setDeviceInfo($devinfo) {
		$this->_devinfo = $devinfo;
	}

	function getLastAnchor($collection) {
		if (isset($this->_anchors[$collection->getName()]))
			return $this->_anchors[$collection->getName()];
		return null;
	}
	
	function setLastAnchor($collection, $anchor) {
		$this->_anchors[$collection->getName()] = $anchor;
	}
	
	/**
	 * Returns the global id given a local (device) id. null if no such id
	 * @return	string
	 */
	function getGlobalId($collection, $luid) {
		assert('is_object($collection)');
		return isset($this->_map[$collection->getName()][trim($luid, './')]) ? 
			$this->_map[$collection->getName()][trim($luid, './')] : null;
	}
	
	function getLocalId($collection, $guid) {
		if (isset($this->_map[$collection->getName()]) && 
			false !== ($luid = array_search($guid, $this->_map[$collection->getName()])))
			return $luid;
		return null;
	}
	
	function setGlobalId($collection, $luid, $guid) {
		assert('is_object($collection)');
		$this->removeGlobalId($collection, $guid);
		$this->_map[$collection->getName()][trim($luid, './')] = $guid;
	}
	
	function removeGlobalId($collection, $guid) {
		if (null !== ($luid = $this->getLocalId($collection, $guid)))
			unset($this->_map[$collection->getName()][$luid]);
	}
	
	function getMappings($collection) {
		return isset($this->_map[$collection->getName()]) ? 
			$this->_map[$collection->getName()] : array();
	}
}
