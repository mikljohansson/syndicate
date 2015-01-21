<?php
require_once 'core/model/DomainContent.class.inc';

class synd_type_text extends SyndType implements DomainContent {
	var $_text = null;
	
	function __construct($text = null) {
		parent::__construct();
		$this->_text = $text;
	}
	
	function toString() {
		return $this->_text;
	}
	
	function setContent($content) {
		$this->_text = $content;
	}
	
	function _view_setContent(Request $request, Template $page) {
		if (null == ($parent = $this->getParent()) || !$parent->isPermitted('write'))
			throw new ForbiddenException();
		$this->setContent($request[0]);
	}

	function acceptIndexer(IndexBuilder $indexer) {
		$indexer->visitFragment($this->_text);
	}
}
