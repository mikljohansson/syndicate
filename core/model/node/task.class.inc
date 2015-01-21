<?php
require_once 'core/model/DatabaseEntity.class.inc';

class synd_node_task extends AbstractDatabaseEntity {
	function __construct($storage, $database, $data) {
		if (null != $data['DATA_CONTENT'])
			$data['DATA_CONTENT'] = unserialize($data['DATA_CONTENT']);
		if (null != $data['DATA_FILES'])
			$data['DATA_FILES'] = unserialize($data['DATA_FILES']);
		parent::__construct($storage, $database, $data);
	}
	
	function __factory() {
		global $synd_user;
		$this->data['NODE_ID'] = $this->classId().'.'.$this->_db->nextId('synd_issue_task');
		$this->data['TS_CREATE'] = $this->data['TS_UPDATE'] = time();
		$this->data['CREATE_NODE_ID'] = $this->data['UPDATE_NODE_ID'] = $synd_user->nodeId;
		parent::__factory();
	}

	function __clone() {
		global $synd_user;
		$this->data['NODE_ID'] = $this->classId().'.'.$this->_db->nextId('synd_issue_task');
		$this->data['TS_CREATE'] = time();
		$this->data['CREATE_NODE_ID'] = $synd_user->nodeId;

		parent::__clone();

		$this->setContent(clone $this->getContent());
	}
	
	function toString() {
		return tpl_chop($this->getDescription(), 50);
	}
	
	static function getEntityQuery(DomainStorage $storage) {
		return self::createQuery($storage, 'synd_issue_task');
	}
	
	function nodeId() {
		return $this->data['NODE_ID'];
	}

	function hasPermission($user, $perm, $arg = null) {
		if ('merge' == $perm && in_array($arg, array('INFO_DURATION','FLAG_PROTECTED')))
			return $this->hasPermission($user, 'write');
		
		// Escalate 'read' and 'write' perms to 'admin' if this note is protected
		if ($this->data['FLAG_PROTECTED'] && ('read' == $perm || 'write' == $perm))
			return parent::hasPermission($user, 'admin', $arg);
		
		return parent::hasPermission($user, $perm, $arg);
	}
	
	function getHandler() {
		$parent = $this->getParent();
		return $parent->getHandler();
	}

	/**
	 * Includes the description as plaintext
	 */
	function getCompositeData() {
		$data = parent::getCompositeData();
		$content = $this->getContent();
		$data['content'] = $content->toString();
		return $data;
	}
	
	/**
	 * Intercepts synd_type_file objects
	 */
	function appendChild($child) {
		if (($child instanceof synd_type_file)) {
			$this->data['DATA_FILES'][$child->nodeId()] = $child;
			$child->setParent($this);
			return $child;
		}
		return parent::appendChild($child);
	}
	
	function validate($data) {
		require_once 'core/lib/SyndDate.class.inc';
		if (isset($data['INFO_DURATION']))
			$data['INFO_DURATION'] = SyndDate::evalDuration($data['INFO_DURATION']);
		if ($this->isNew() && (!isset($data['INFO_DURATION']) || null == $data['INFO_DURATION']))
			$data['INFO_DURATION'] = 20;

		return parent::validate($data);
	}
	
	function merge($data) {
		require_once 'core/lib/SyndDate.class.inc';
		if (isset($data['INFO_DURATION'])) {
			$data['INFO_DURATION'] = SyndDate::evalDuration($data['INFO_DURATION']);
			$data['FLAG_PROTECTED'] = (int)!empty($data['FLAG_PROTECTED']);
		}
		
		if (isset($data['content'])) {
			$content = $this->getContent();
			$content->setContent($data['content']);
			unset($data['content']);
		}

		parent::merge($data);
	}

	function acceptIndexer(IndexBuilder $indexer) {
		parent::acceptIndexer($indexer);
		
		$this->getContent()->acceptIndexer($indexer);
//		$this->getCreator()->acceptIndexer($indexer);
		
		foreach ($this->getFiles() as $file)
			$indexer->visitFile($file->path());
	}
	
	function setParent($parent) {
		$this->data['PARENT_NODE_ID'] = $parent->nodeId;
	}
	
	function getParent() {
		if (null === ($parent = $this->_storage->getInstance($this->data['PARENT_NODE_ID'])))
			$parent = parent::getParent();
		return $parent;
	}

	function getUpdateTime() {
		return $this->data['TS_UPDATE'];
	}	

	function delete() {
		$content = $this->getContent();
		$content->delete();
		parent::delete();
	}
	
	function getIssue() {
		return $this->getParent();
	}

	function getCreator() {
		if (null == ($creator = SyndNodeLib::getInstance($this->data['CREATE_NODE_ID'])))
			$creator = SyndNodeLib::getInstance('user_null.null');
		return $creator;
	}
	
	function setCreator($creator) {
		$this->data['CREATE_NODE_ID'] = $creator->nodeId;
		if (empty($this->data['UPDATE_NODE_ID']))
			$this->data['UPDATE_NODE_ID'] = $creator->nodeId;
	}
	
	/**
	 * Returns the text content object
	 * @return	DomainContent
	 */
	function getContent() {
		if (null == $this->data['DATA_CONTENT'])
			$this->setContent(SyndType::factory('text'));
		return $this->data['DATA_CONTENT'];
	}
	
	/**
	 * Set the text content object
	 * @param	DomainContent
	 */
	function setContent($content) {
		$content->setParent($this);
		$this->data['DATA_CONTENT'] = $content;
	}
	
	/**
	 * Returns the task description
	 * @return	string
	 */
	function getDescription() {
		$content = $this->getContent();
		return $content->toString();
	}
	
	/**
	 * Sets the issue description
	 * @param	string	Issue description
	 */
	function setDescription($description) {
		$content = $this->getContent();
		$content->setContent($description);
	}
	
	/**
	 * Used from full_edit.tpl
	 */
	function _view_setDescription(Request $request, Template $page) {
		if (!$this->isPermitted('write'))
			throw new ForbiddenException();
		if (!isset($request['description']))
			throw new InvalidArgumentException();
		$this->setDescription($request['description']);
	}

	/**
	 * Loads the content and file instances
	 */
	function _hook_loadInstance($id) {
		$content = $this->getContent();
		if ($content->getInstanceId() == $id)
			return $content;

		foreach (array_keys($files = $this->getFiles()) as $key) {
			if ($files[$key]->getInstanceId() == $id)
				return $files[$key];
		}
	
		$result = null;
		return $result;
	}

	/**
	 * Returns the files attached to this note
	 * @see		synd_type_file
	 * @return	array	Array of synd_type_file objects
	 */
	function getFiles() {
		if (null != $this->data['DATA_FILES'])
			return $this->data['DATA_FILES'];
		return array();
	}
	
	/**
	 * Returns a file given its id
	 * @see		getFiles();
	 * @return	synd_type_file	Return null if not found
	 */
	function getFile($id) {
		if (isset($this->data['DATA_FILES'][$id]))
			return $this->data['DATA_FILES'][$id];
		$result = null;
		return $reuslt;
	}
	
	/**
	 * Deletes a file given its id
	 * @see		getFiles();
	 * @param	string	Key of file
	 */
	function unlink($id) {
		if (!isset($this->data['DATA_FILES'][$id]) || !$this->data['DATA_FILES'][$id]->delete()) 
			return false;
		unset($this->data['DATA_FILES'][$id]);
		return true;
	}
	
	function _view_unlink(Request $request, Template $page) {
		if (!$this->isPermitted('write'))
			throw new ForbiddenException();
		if (!isset($request[2]))
			return null;
		
		if (isset($request['confirm'])) {
			if ($this->unlink($request[2]))
				$this->_save();
		}
		else {
			if (null === ($file = $this->getFile($request[2])))
				throw new NotFoundException();
			return array('content' => $page->fetchnode($this,'confirm_delete_file.tpl',array('file' => $file)));
		}
	}

	/**
	 * Returns the duration in seconds.
	 * @return	integer
	 */
	function getDuration() {
		return $this->data['INFO_DURATION'] * 60;
	}
	
	/**
	 * Sets the duration in seconds
	 * @param	integer	Number of seconds
	 */
	function setDuration($duration) {
		if ($this->data['INFO_DURATION'] != round($duration/60))
			$this->data['INFO_DURATION'] = round($duration/60);
	}
	
	function getCreateTime() {
		return $this->data['TS_CREATE'];
	}
	
	function _view_setProtected(Request $request, Template $page) {
		if (!$this->isPermitted('admin'))
			throw new ForbiddenException();
		$this->data['FLAG_PROTECTED'] = (int)!empty($request[2]);
		$this->_save();
	}

	function _view_forward(Request $request, Template $page) {
		$issue = $this->getParent();
		if (!$issue->isPermitted('write'))
			throw new ForbiddenException();
		
		return array('content' => $page->fetchnode($issue,'mail/forward.tpl', array(
			'content'	=> $this->getContent(),
			'files'		=> $this->getFiles())));
	}

	function _view_reply(Request $request, Template $page) {
		$issue = $this->getParent();
		if (!$issue->isPermitted('write'))
			throw new ForbiddenException();
		
		return array('content' => $page->fetchnode($issue,'mail/reply.tpl', array(
			'to'		=> $this->getCreator()->getEmail(),
			'content'	=> $this->getContent(), 
			'files'		=> $this->getFiles())));
	}
	
	function _entity_onpreflush() {
		global $synd_user;
		parent::_entity_onpreflush();
		$this->data['TS_UPDATE'] = time();
		$this->data['UPDATE_NODE_ID'] = $synd_user->nodeId;
	}

	/**
	 * Deletes files from disk
	 */
	function _node_onpredelete() {
		SyndLib::invoke($this->getFiles(),'delete');
		parent::_node_onpredelete();
	}
}
