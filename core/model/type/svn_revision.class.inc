<?php
require_once 'core/lib/vendor/SVN.class.inc';
require_once 'core/model/DomainContent.class.inc';

class synd_type_svn_revision extends SyndType implements DomainContent {
	var $_revision = null;

	function __construct($revision) {
		parent::__construct();
		$this->_revision = $revision;
	}

	function toString() {
		return $this->_revision;
	}
	
	function setContent($revision) {
		$this->_revision = $revision;
	}
	
	function getRevision() {
		if (null != ($uri = $this->getAttribute('svn_repository_location'))) {
			$repository = new SvnWebDavRepository($uri);
			return $revision = $repository->getRevision($this->_revision);
		}
		return new SvnNullRevision();
	}

	function acceptIndexer(IndexBuilder $indexer) {
		$indexer->visitFragment($this->getRevision()->getComment());
	}
}
