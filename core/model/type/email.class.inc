<?php
require_once 'core/lib/Mime.class.inc';
require_once 'core/model/DomainContent.class.inc';

class synd_type_email extends SyndType implements DomainContent {
	/**
	 * Wrapped MIME message
	 * @var	object	SyndMIME
	 * @access	private
	 */
	var $_message = null;
	
	/**
	 * User specified content
	 * @var	string
	 * @access	private
	 */
	var $_content = null;
	
	/**
	 * @param	SyndMIME	MIME message to wrap
	 */
	function __construct($message) {
		parent::__construct();
		$this->_message = $message;
	}
	
	function toString() {
		if (null !== $this->_content)
			return $this->_content;
		return $this->_message->getContent();
	}
	
	function setContent($content) {
		$this->_content = $content;
	}

	function _view_setContent(Request $request, Template $page) {
		if (null == ($parent = $this->getParent()) || !$parent->isPermitted('write'))
			throw new ForbiddenException();
		$this->setContent($request[0]);
	}
	
	/**
	 * Extracts the email address from the From: header
	 * @return	string
	 */
	function getSender() {
		$from = $this->_message->getHeader('From');
		if (preg_match('/<([\w\.+\- ]+@[\w\.\-]+)>/', $from, $matches))
			return $matches[1];
		return trim($from);
	}
	
	/**
	 * Extracts the sender name and email
	 * @return	string
	 */
	function getSenderName() {
		$from = $this->_message->getHeader('From');
		if (preg_match('/([^<]+)</', $from, $matches))
			return trim($matches[1]);
		return trim($from);
	}
	
	/**
	 * Extracts the email address from the Reply-To: header
	 * @return	string
	 */
	function getReplyTo() {
		$replyto = $this->_message->getHeader('Reply-To');
		if (preg_match('/<([\w\.+\- ]+@[\w\.\-]+)>/', $replyto, $matches))
			return $matches[1];
		return trim($replyto);
	}

	/**
	 * Extracts the email address from the To: header
	 * @return	string
	 */
	function getReceiver() {
		$to = $this->_message->getHeader('To');
		if (preg_match('/<([\w\.+\- ]+@[\w\.\-]+)>/', $to, $matches))
			return $matches[1];
		return trim($to);
	}
	
	/**
	 * Returns addresses from the To, Cc: and Bcc: fields
	 * @return	array
	 */
	function getReceivers() {
		return array_merge(
			$this->_extract($this->_message->getHeader('To')),
			$this->_extract($this->_message->getHeader('Cc')),
			$this->_extract($this->_message->getHeader('Bcc')));
	}

	/**
	 * Returns addresses from the Cc: and Bcc: fields
	 * @return	array
	 */
	function getCarbonCopy() {
		return array_merge(
			$this->_extract($this->_message->getHeader('Cc')),
			$this->_extract($this->_message->getHeader('Bcc')));
	}

	/**
	 * @access	private
	 */
	function _extract($header) {
		preg_match_all('/[\w\.+\- ]+@[\w\.\-]+/', $header, $matches);
		return array_map('trim', $matches[0]);
	}
	
	/**
	 * Returns the subject
	 * @return	string
	 */
	function getSubject() {
		return $this->_message->getHeader('Subject');
	}
	
	function getBody() {
		return $this->_message->getContent();
	}
	
	/**
	 * Returns the mime message
	 * @return	SyndMIME
	 */
	function getMessage() {
		return $this->_message;
	}
	
	/**
	 * Hides quoted message blocks using javascript
	 * @param	Template	Page template being processed
	 * @param	string		Raw text to process
	 * @return	string
	 */
	function hideQuotedText(Template $page, $text) {
		profiler_enter(__METHOD__);
		/*$text = preg_replace_callback(
			'/(?:(?:^|\n)(?: *(?:>|&gt;) *)+(?>[^\r\n]*)(?:(?:\s|<br \/>)+(?=\n(?:>|&gt;)))?){15,249}/S', 
			Delegate::factory(array($this, '_callback_format_quoted_block'), array($page)), $text);*/
		for ($i=0; $i<4; $i++)
			$text = preg_replace('/(?<=^|\n)(?:[ \t]|&nbsp;)*(?:(?:>|&gt;)[ \t]*)((?>[^\r\n]*)(?:(?:\s|<[\w \/="]+>)+(?=\n(>|&gt;)))?)/S', "<span class=\"Quote\">\r\n\\1</span>", $text);
		profiler_leave(__METHOD__);
		return $text;
	}
	
	/**
	 * @access	private
	 */
	function _callback_format_quoted_block(Template $page, $matches) {
		return $page->fetchnode(
			$this, 'quoted_message_block.tpl', 
			array('text' => $matches[0], 'lines' => substr_count($matches[0],"\n")));
	}
	
	function _view_source(Request $request, Template $page) {
		if (!$this->getParent()->isPermitted('read'))
			throw new ForbiddenException();
		return array('content' => $page->fetchnode($this,'source.tpl'));
	}

	function acceptIndexer(IndexBuilder $indexer) {
		$indexer->visitFragment($this->_message->acceptIndexer($indexer));
	}
}
