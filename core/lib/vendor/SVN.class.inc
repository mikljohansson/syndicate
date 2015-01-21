<?php
require_once 'core/lib/HttpConnection.class.inc';

/**
 * SVN WebDAV repository
 * @abstract
 * @access		public
 * @package		synd.core.lib
 */
class SvnWebDavRepository {
	/**
	 * @access	protected
	 */
	var $_uri = null;

	/**
	 * @access	private
	 */
	var $_connection = null;
	
	/**
	 * @param	string	Repository base URI
	 */
	function SvnWebDavRepository($uri) {
		$this->_uri = $uri;
	}
	
	/**
	 * @access	protected
	 * @return	HttpConnection
	 */
	function getConnection() {
		if (null === $this->_connection)
			$this->_connection = new HttpConnection($this->_uri);
		return $this->_connection;
	}
	
	/**
	 * Fetches a revision from repository
	 * @param	integer	Revision number
	 * @return	SvnRevision
	 */
	function getRevision($revision) {
		$xml = "
			<S:log-report xmlns:S=\"svn:\">
				<S:start-revision>$revision</S:start-revision>
				<S:end-revision>$revision</S:end-revision>
				<S:limit>100</S:limit>
				<S:discover-changed-paths/>
				<S:path></S:path>
			</S:log-report>";
		$connection = $this->getConnection();
		$response = $connection->postRequest($this->_uri, $xml, 'text/xml', 'REPORT');
		if (null == $response)
			return new SvnNullRevision();
		return SvnRevision::parse($this->_uri, $response);
	}
}

define('SVN_PARSE_MODIFY', 1);
define('SVN_PARSE_DELETE', 2);
define('SVN_PARSE_ADD', 3);
define('SVN_PARSE_COMMENT', 4);
define('SVN_PARSE_CREATOR', 5);
define('SVN_PARSE_DATE', 6);
define('SVN_PARSE_VERSION', 7);

/**
 * SVN revision
 * @access		public
 * @package		synd.core.lib
 */
class SvnRevision {
	/**
	 * @access	protected
	 */
	var $_repository = null;

	/**
	 * @access	private
	 */
	var $_stack = null;

	/**
	 * @access	private
	 */
	var $_revision = null;

	/**
	 * @access	private
	 */
	var $_creator = null;

	/**
	 * @access	private
	 */
	var $_modifications = null;
	
	/**
	 * @access	private
	 */
	var $_comment = null;
	
	/**
	 * @access	private
	 */
	var $_created = null;

	/**
	 * @access	private
	 */
	var $_tmp = null;
	
	function SvnRevision($repository) {
		$this->_repository = $repository;
	}
	
	/**
	 * Parses an XML document into a revision
	 * @param	string	Base URI to repository
	 * @param	string	XML reponse to parse
	 * @return	SvnRevision
	 */
	static function parse($repository, $xml) {
		$revision = new SvnRevision($repository);
		$revision->_stack = array();
		
		$parser = xml_parser_create('ISO-8859-1');
		xml_parser_set_option($parser, XML_OPTION_CASE_FOLDING, false);
		xml_parser_set_option($parser, XML_OPTION_SKIP_WHITE, true);

		xml_set_element_handler($parser, 
			array($revision, '_callback_enter'),
			array($revision, '_callback_leave'));
		xml_set_character_data_handler($parser, 
			array($revision, '_callback_cdata'));

		xml_parse($parser, $xml, true);
		xml_parser_free($parser);
		
		$revision->_stack = null;
		return $revision;
	}

	function isNull() {
		return false;
	}

	/**
	 * Returns the name of the creator of this revision
	 * @return	string
	 */
	function getCreator() {
		return $this->_creator;
	}
	
	/**
	 * Returns the modifcations in this revision
	 * @return	array
	 */
	function getModifications() {
		return (array)$this->_modifications;
	}
	
	/**
	 * Returns the comment for this revision
	 * @return	string
	 */
	function getComment() {
		return $this->_comment;
	}
	
	/**
	 * Returns the timestamp this revision was created
	 * @return	integer
	 */
	function getCreateTime() {
		return $this->_created;
	}
	
	/**
	 * @access	private
	 */
	function _callback_enter($parser, $name, $attributes) {
		switch ($name) {
			case 'D:version-name':
				array_push($this->_stack, SVN_PARSE_VERSION);
				break;
			case 'S:modified-path':
				array_push($this->_stack, SVN_PARSE_MODIFY);
				$this->_tmp = new SvnRevisionModify($this->_repository, $this->_revision);
				break;
			case 'S:deleted-path':
				array_push($this->_stack, SVN_PARSE_DELETE);
				$this->_tmp = new SvnRevisionDelete($this->_repository, $this->_revision);
				break;
			case 'S:added-path':
				array_push($this->_stack, SVN_PARSE_ADD);
				$this->_tmp = new SvnRevisionAdd($this->_repository, $this->_revision);
				break;
			case 'D:comment':
				array_push($this->_stack, SVN_PARSE_COMMENT);
				break;
			case 'D:creator-displayname':
				array_push($this->_stack, SVN_PARSE_CREATOR);
				break;
			case 'S:date':
				array_push($this->_stack, SVN_PARSE_DATE);
				break;
		}
	}
	
	/**
	 * @access	private
	 */
	function _callback_leave($parser, $name) {
		switch (end($this->_stack)) {
			case SVN_PARSE_MODIFY:
			case SVN_PARSE_DELETE:
			case SVN_PARSE_ADD:
				$this->_modifications[] = $this->_tmp;
				unset($this->_tmp);
				$this->_tmp = null;
				break;
			case SVN_PARSE_DATE:
				$this->_created = strtotime(preg_replace('/\.\d+Z$/', 'Z', str_replace('T',' ',$this->_created)));
				break;
		}
		
		array_pop($this->_stack);
	}

	/**
	 * @access	private
	 */
	function _callback_cdata($parser, $cdata) {
		switch (end($this->_stack)) {
			case SVN_PARSE_VERSION:
				$this->_revision = trim($cdata);
				break;
			case SVN_PARSE_MODIFY:
			case SVN_PARSE_DELETE:
			case SVN_PARSE_ADD:
				if ('' != ($content = trim($cdata)) || null != $this->_tmp->_path)
					$this->_tmp->_path .= "\n" == $cdata ? "\r\n" : $cdata;
				break;
			case SVN_PARSE_COMMENT:
				if ('' != ($content = trim($cdata)) || null != $this->_comment)
					$this->_comment .= "\n" == $cdata ? "\r\n" : $cdata;
				break;
			case SVN_PARSE_CREATOR:
				if ('' != ($content = trim($cdata)) || null != $this->_creator)
					$this->_creator .= "\n" == $cdata ? "\r\n" : $cdata;
				break;
			case SVN_PARSE_DATE:
				$this->_created .= trim($cdata);
				break;
		}
	}
}

class SvnNullRevision extends SvnRevision {
	function SvnNullRevision() {}

	function isNull() {
		return true;
	}
}

/**
 * SVN revision modification
 * @abstract
 * @access		public
 * @package		synd.core.lib
 */
class SvnRevisionModify {
	/**
	 * @access	protected
	 */
	var $_repository = null;

	/**
	 * @access	protected
	 */
	var $_revision = null;
	
	/**
	 * @access	protected
	 */
	var $_path = null;

	function SvnRevisionModify($repository, $revision, $path = null) {
		$this->_repository = $repository;
		$this->_revision = $revision;
		$this->_path = $path;
	}

	/**
	 * Returns the name of this modification
	 * @return	string
	 */
	function getTitle() {
		return 'Modified';
	}
	
	/**
	 * Returns the path to modifed file
	 * @return	string
	 */
	function getPath() {
		return $this->_path;
	}
	
	/**
	 * Returns URI of this modification
	 * @param	string	Base URI to repository
	 * @param	string	Base URI to ViewVC installation
	 * @return	string
	 */
	function getLocation($viewvc = null) {
		if (null !== $viewvc)
			return rtrim($viewvc,'/').$this->_path.'?pathrev='.$this->_revision.'&r1='.($this->_revision-1).'&r2='.$this->_revision;
		return rtrim($this->_repository,'/').$this->_path;
	}
}

class SvnRevisionDelete extends SvnRevisionModify {
	function SvnRevisionDelete($repository, $revision, $path = null) {
		parent::SvnRevisionModify($repository, $revision, $path);
	}

	function getTitle() {
		return 'Deleted';
	}

	function getLocation($viewvc = null) {
		if (null !== $viewvc)
			return rtrim($viewvc,'/').$this->_path.'?view=markup&pathrev='.($this->_revision-1);
		return rtrim($this->_repository,'/').$this->_path;
	}
}

class SvnRevisionAdd extends SvnRevisionModify {
	function SvnRevisionAdd($repository, $revision, $path = null) {
		parent::SvnRevisionModify($repository, $revision, $path);
	}

	function getTitle() {
		return 'Added';
	}

	function getLocation($viewvc = null) {
		if (null !== $viewvc)
			return rtrim($viewvc,'/').$this->_path.'?view=markup&pathrev='.$this->_revision;
		return rtrim($this->_repository,'/').$this->_path;
	}
}
