<?php
require_once 'core/model/node/node.class.inc';

class synd_node_page extends synd_node_node {
	/**
	 * @access	private
	 */
	var $_children = null;

	/**
	 * Caches the ids of direct sub chapters
	 * @access	private
	 */
	var $_chapters = null;
	
	/**
	 * Caches the ids of files
	 * @access	private
	 */
	var $_files = null;

	/**
	 * Caches the ids of all questionnaire in this branch
	 * @access	private
	 */
	var $_questionnaires = null;

	/**
	 * Caches the ids of all sub chapters with FLAG_PROGRESS set
	 * @access	private
	 */
	var $_testChapters = null;

	function __construct($storage, $database, $data) {
		if (null != $data['INFO_BODY'])
			$data['INFO_BODY'] = unserialize($data['INFO_BODY']);
		parent::__construct($storage, $database, $data);
	}
	
	function __factory() {
		parent::__factory();
		$this->assign('INFO_BODY', SyndType::factory('html'));
	}

	function __clone() {
		// Get children before parent::__clone() since it changes the nodeId
		$children = $this->getChildren();

		parent::__clone();

		$body = $this->getBody();
		$this->assign('INFO_BODY', clone $body);
		
		// Clone must be written to db ahead of children to satisfy 
		// their parent_node_id foreign key
		$this->save();
		
		foreach ($children as $child) {
			$child = $child->copy();
			$child->setParent($this);
			$child->save();
		}
	}
	
	function __sleep() {
		$vars = parent::__sleep();
		unset($vars[array_search('_children', $vars)]);
		return $vars;
	}
	
	static function getEntityQuery(DomainStorage $storage) {
		$query = parent::getEntityQuery($storage);
		$query->join('synd_node_page');
		return $query;
	}
	
	function hasPermission($user, $perm, $arg = null) {
		global $synd_user;

		switch ($perm) {
			case 'read':
				switch ($arg) {
					case null:
					case 'full_view_results.tpl':
					case 'full_view_print.tpl':
						return parent::hasPermission($user, 'read');

					case 'full_view_progress.tpl':
						return $this->hasPermission($user, 'progress_check');
					case 'full_view_diagnostic.tpl':
						return $this->hasPermission($user, 'diagnostic_test');

					default:
						return false;
				}
				break;
			
			case 'write':
				$creator = $this->getCreator();
				if ($creator->hasAuthorization($user, $this))
					return true;
				break;
			
			case 'append':
				return ($arg instanceof synd_node_page) && $this->hasPermission($user, 'write');
			case 'remove':
				return $this->hasPermission($user, 'write');
			
			case 'progress_check':
				return $this->hasProgressCheck() && !$synd_user->isNull() && $this->hasPermission($user, 'read');
			case 'diagnostic_test':
				return $this->hasDiagnosticTest() && !$synd_user->isNull() && $this->hasPermission($user, 'read');

			case 'statistics':
				$parent = $this->getParent();
				return $parent->hasPermission($user, $perm, $arg);
		}

		if ('merge' == $perm && in_array($arg, array(
			'INFO_HEAD','INFO_DESC','INFO_BODY','FLAG_PROGRESS','FLAG_DIAGNOSTIC')))
			return $this->hasPermission($user, 'write');

		return parent::hasPermission($user, $perm, $arg);
	}
	
	function delete() {
		SyndLib::invoke($this->getChildren(),'delete');
		parent::delete();
	}

	function hasProgressCheck() {
		return $this->data['FLAG_PROGRESS'];
	}

	function hasDiagnosticTest() {
		return $this->data['FLAG_DIAGNOSTIC'];
	}
	
	function toString() {
		if (null != $this->data['INFO_HEAD'])
			return strip_tags($this->data['INFO_HEAD']);
		return SyndLib::translate('No title');
	}

	function merge($data) {
		if (isset($data['INFO_HEAD']) && !isset($data['FLAG_PROGRESS']))
			$data['FLAG_PROGRESS'] = 0;
		if (isset($data['INFO_HEAD']) && !isset($data['FLAG_DIAGNOSTIC']))
			$data['FLAG_DIAGNOSTIC'] = 0;
		
		if (isset($data['INFO_BODY'])) {
			$this->setBody($data['INFO_BODY']);
			unset($data['INFO_BODY']);
		}
		
		parent::merge($data);
		
		$parent = $this->getParent();
		if ($this->isNew() && !$parent->isNull() && count($parent->getChildren()))
			$this->data['INFO_WEIGHT'] = max(SyndLib::invoke($parent->getChildren(),'getWeight'))+1;
	}
	
	function getCourse() {
		$parent = $this->getParent();
		if (!$parent->isNull())
			return $parent->getCourse();
		return $parent;
	}
	
	function getCreator() {
		if (null === ($creator = SyndNodeLib::getInstance($this->data['CREATE_NODE_ID'])))
			$creator = SyndNodeLib::getInstance('user_null.null');
		return $creator;
	}
	
	/**
	 * Returns the branch to this node indexed by nodeId
	 * @return	array
	 */
	function getBranch() {
		$parent = $this->getParent();
		$branch = $parent->isNull() ? array() : $parent->getBranch();
		if (!isset($branch[$this->nodeId]))
			$branch[$this->nodeId] = $this;
		return $branch;
	}
	
	function getHandler() {
		return 'course';
	}
	
	function getRevisions() {
		/*
		$sql = "
			SELECT r.node_id, r.ts_create FROM synd_revision r
			WHERE r.parent_node_id = ".$this->_db->quote($this->nodeId)." 
			ORDER BY r.ts_create DESC";
		return $this->_storage->getInstances($this->_db->getCol($sql,0,0,5));
		*/
		return array();
	}
	
	function getQuestions() {
		$questions = array();
		foreach (array_keys($children = $this->getChapters()) as $key)
			$questions = SyndLib::array_merge_assoc($questions, $children[$key]->getQuestions());
		return $questions;
	}

	function createAttempt() {
		$attempt = SyndNodeLib::factory('attempt');
		$attempt->setParent($this);
		foreach (array_keys($questions = $this->getQuestions()) as $key)
			$attempt->addQuestion($questions[$key]);
		return $attempt;
	}
	
	/** 
	 * Creates a progress check attempt.
	 * @return	synd_node_attempt
	 */
	function createProgressAttempt() {
		$attempt = SyndNodeLib::factory('attempt');
		$attempt->setParent($this);

		// Randomize the order of the questionnaires
		$pages = $this->getQuestionnaires();
		$keys = array_keys($pages);
		srand();
		shuffle($keys);
		
		// Walk through questionnairs and get max 6 questions from each
		foreach ($keys as $key) {
			foreach (array_keys($questions = $pages[$key]->getProgressQuestions(6)) as $key2) {
				// Break when the maximum 30 questions is reached
				if (count($attempt->getQuestions()) >= 30)
					break 2;
				$attempt->addQuestion($questions[$key2]);
			}
		}
		
		return $attempt;
	}
	
	function createDiagnosticTest() {
		$test = array();
		foreach (array_keys($children = $this->getChapters()) as $key) {
			if (null != ($attempt = $children[$key]->createDiagnosticAttempt()))
				$test[] = $attempt;
		}
		return $test;
	}

	/** 
	 * Creates a diagnostic test attempt.
	 * @return	synd_node_attempt
	 */
	function createDiagnosticAttempt() {
		// Get all diagnostic questions in this tree
		if (!count($questions = $this->getDiagnosticQuestions())) {
			$result = null;
			return $result;
		}
		
		$attempt = SyndNodeLib::factory('attempt');
		$attempt->setParent($this);

		foreach (array_keys($questions) as $key)
			$attempt->addQuestion($questions[$key]);
		return $attempt;
	}
	
	function getDiagnosticQuestions() {
		$questions = array();
		foreach (array_keys($chapters = $this->getChapters()) as $key) {
			$questions = SyndLib::array_merge_assoc($questions, 
				$chapters[$key]->getDiagnosticQuestions());
		}
		return $questions;
	}

	function _createRevision() {
		/*
		$revision = SyndNodeLib::factory('revision');
		$revision->setParent($this);
		$revision->save();
		*/
	}

	/**
	 * Returns direct children of this node
	 * @return	array	array(object synd_node_node)
	 */
	function getChildren() {
		if (null === $this->_children)
			$this->_children = SyndLib::sort(parent::getChildren(), 'getWeight');
		return $this->_children;
	}
	
	/**
	 * Returns all child questionnaires, recurses the tree
	 * @return	array
	 */
	function getQuestionnaires() {
		if (null !== $this->_questionnaires)
			return SyndNodeLib::getInstances($this->_questionnaires);
		
		$questionnaires = array();
		foreach (array_keys($chapters = $this->getChapters()) as $key) {
			$questionnaires = SyndLib::array_merge_assoc(
				$questionnaires, $chapters[$key]->getQuestionnaires());
		}
		
		$this->_questionnaires = array_keys($questionnaires);
		$this->_node_onchange_internal();

		return $questionnaires;
	}
	
	/**
	 * Returns direct sub chapters of this node
	 * @return	array	array(object synd_node_page)
	 */
	function getChapters() {
		if (null === $this->_chapters) {
			$sql = "
				SELECT n.node_id, p.info_weight, p.info_head 
				FROM synd_node n, synd_node_page p
				WHERE 
					n.node_id = p.node_id AND
					n.parent_node_id = ".$this->_db->quote($this->nodeId)."
				ORDER BY p.info_weight, p.info_head";
			$this->_chapters = (array)$this->_db->getCol($sql);
			$this->_node_onchange_internal();
		}
		return $this->_storage->getInstances($this->_chapters);
	}
	
	function getFiles() {
		if (null === $this->_files) {
			$sql = "
				SELECT n.node_id, f.info_weight, f.info_head 
				FROM synd_node n, synd_node_file f
				WHERE 
					n.node_id = f.node_id AND
					n.parent_node_id = ".$this->_db->quote($this->nodeId)."
				ORDER BY f.info_weight, f.info_head";
			$this->_files = (array)$this->_db->getCol($sql);
			$this->_node_onchange_internal();
		}
		return $this->_storage->getInstances($this->_files);
	}

	function hasChapters() {
		if (null !== $this->_chapters)
			return count($this->_chapters);
		return count($this->getChapters());
	}
	
	/**
	 * Recurses the tree and returns all sub chapters with progress checks.
	 * @return	array	array(object synd_node_page)
	 */
	function getProgressChapters() {
		if (null !== $this->_testChapters)
			return SyndNodeLib::getInstances($this->_testChapters);
		
		$testChapters = array();
		foreach (array_keys($chapters = $this->getChapters()) as $key) {
			if ($chapters[$key]->hasProgressCheck())
				$testChapters[$chapters[$key]->nodeId] = $chapters[$key];
			$testChapters = SyndLib::array_merge_assoc(
				$testChapters, $chapters[$key]->getProgressChapters());
		}
		
		$this->_testChapters = array_keys($testChapters);
		$this->_node_onchange_internal();
		
		return $testChapters;
	}

	function getAttempts() {
		$sql = "
			SELECT a.node_id, a.ts_create FROM synd_attempt a
			WHERE a.parent_node_id = ".$this->_db->quote($this->nodeId)." 
			ORDER BY a.ts_create DESC";
		return $this->_storage->getInstances($this->_db->getCol($sql));
	}

	function getAttemptCount() {
		$sql = "
			SELECT COUNT(1) FROM synd_attempt a
			WHERE a.parent_node_id = ".$this->_db->quote($this->nodeId);
		return $this->_db->getOne($sql);
	}

	function getProgressAttempts($client) {
		SyndNodeLib::loadClass('attempt');
		return synd_node_attempt::getAttempts($this, $client);
	}

	/**
	 * Recurses the tree and returns a clients attempts.
	 * @return	array	array(object synd_node_attempt)
	 */
	function getClientAttempts($client) {
		$attempts = array();
		if ($this->hasProgressCheck())
			$attempts = $this->getProgressAttempts($client);
		
		foreach (array_keys($chapters = $this->getProgressChapters()) as $key) {
			$attempts = SyndLib::array_merge_assoc($attempts, 
				$chapters[$key]->getClientAttempts($client));
		}
		
		return $attempts;
	}
	
	function getBestProgressAttempt($client) {
		if (!$this->hasProgressCheck())
			return null;
		
		require_once 'core/model/node/attempt.class.inc';
		return synd_node_attempt::getBestAttempt($this, $client);
	}
	
	function getWorstAttempt($client) {
		require_once 'core/model/node/attempt.class.inc';
		$progress = null;
		$current = null;
		
		foreach (array_keys($chapters = $this->getChapters()) as $key) {
			if (($chapters[$key]->hasProgressCheck() || $chapters[$key]->hasDiagnosticTest()) &&
				null != ($attempt = synd_node_attempt::getBestAttempt($chapters[$key], $client))) {
				if (null === $progress || $attempt->getProgress() < $progress) {
					$progress = $attempt->getProgress();
					$current = $attempt;
				}
			}
		}
		
		return $current;
	}
	
	function insertBefore($child, $before = null) {
		if (false === ($child = parent::insertBefore($child, $before)))
			return false;
		
		$list = $this->getChildren();
		unset($list[$child->nodeId]);
		
		if (null != $before && isset($list[$before->nodeId])) {
			// Weight of "before"
			$newWeight = $list[$before->nodeId]->getWeight();
			$child->setWeight($newWeight);
			
			// Move slice of nodes after new child down one step
			$slice = array_slice(array_keys($list), 
				array_search($before->nodeId, array_keys($list)));
			foreach ($slice as $key)
				$list[$key]->setWeight(++$newWeight);
		}
		else {
			$child->setWeight(count($list) ? 
				max(SyndLib::invoke($list,'getWeight'))+1 : 0);
		}
		
		$list[$child->nodeId] = $child;
		$list = SyndLib::sort($list, 'getWeight');
		
		$this->_children = $list;
		$this->_chapters = null;
		$this->_questionnaires = null;
		$this->_testChapters = null;

		return $child;
	}
	
	function getWeight() {
		return $this->data['INFO_WEIGHT'];
	}
	
	function setWeight($weight) {
		$this->data['INFO_WEIGHT'] = $weight;
		$this->_save();
	}
	
	function getPage() {
		return $this;
	}

	function getPageNumber() {
		$parent = $this->getParent();
		if (!($parent instanceof synd_node_page)) 
			return null;

		$i = 0;
		foreach (array_keys($children = $parent->getChapters()) as $key) {
			if ($children[$key]->nodeId == $this->nodeId) break;
			if (get_class($children[$key]) == 'synd_node_page') $i++;
		}

		return trim($parent->getPageNumber().'.'.($i+1),'.');
	}
	
	function getTitle() {
		return $this->data['INFO_HEAD'];
	}

	function setTitle($title) {
		$this->data['INFO_HEAD'] = $title;
		$this->_save();
	}
	
	function getDescription() {
		return $this->data['INFO_DESC'];
	}

	function setDescription($description) {
		$this->data['INFO_DESC'] = $description;
		$this->_save();
	}

	function setBody($text) {
		$body = $this->getBody();
		if ($body->toString() != $text) {
			$body->importText($text);
			$this->_save();
			$this->_createRevision();
		}
	}
	
	/**
	 * @deprecated	Use getContent()
	 */
	function getBody() {
		return $this->getContent();
	}
	
	function getContent() {
		if (null == $this->data['INFO_BODY']) {
			$this->data['INFO_BODY'] = SyndType::factory('html');
			$this->data['INFO_BODY']->setParent($this);
			$this->_save();
		}
		return $this->data['INFO_BODY'];
	}
	
	function getText() {
		if (null != $this->data['INFO_BODY'])
			return $this->data['INFO_BODY']->toString();
		return null;
	}

	function getPrevious() {
		$parent = $this->getParent();
		if (!$parent->isNull()) {
			if (null !== ($sibling = SyndLib::array_prev($parent->getChapters(), $this->nodeId)))
				return $sibling->getLast();
			return $parent;
		}
		$result = null; 
		return $result;
	}
	
	function getImplodedTree() {
		$list = array($this->nodeId => $this);
		foreach (array_keys($chapters = $this->getChapters()) as $key)
			$list = SyndLib::array_merge_assoc($list, $chapters[$key]->getImplodedTree());
		return $list;
	}

	function getNext($key = null) {
		if (null != ($next = SyndLib::array_next($this->getChapters(), $key)))
			return $next;
		$parent = $this->getParent();
		if (!$parent->isNull())
			return $parent->getNext($this->nodeId);
		$result = null;
		return $result;
	}

	function getLast() {
		if (count($chapters = $this->getChapters()))
			return SyndLib::end($chapters)->getLast();
		return $this;
	}
	
	function getIndexDocument($index) {
		$body = $this->getBody();
		return $index->createComposite(array(
			$index->createFragment($this->toString()),
			$index->createFragment($this->getDescription()),
			$index->createFragment($body->toString(),null,0.8),
			));
	}
	
	function _metaAttach() {
		SyndLib::attachHook('html_meta_description', array($this, '_callback_html_meta_description'));
	}
	
	function _callback_html_meta_description(&$result) {
		$body = $this->getBody();
		$result .= tpl_chop(preg_replace('/\s+/s', ' ', strip_tags($body->toString())),100);
	}
	
	function _view_newFile(Request $request, Template $page) {
		if (!$this->isPermitted('write'))
			throw new ForbiddenException();
		$file = $this->appendChild($this->_storage->factory('file'));
		return array('redirect' => tpl_view_jump($file->getHandler(),'edit',$file->nodeId));
	}

	/**
	 * Loads the content instances
	 */
	function _hook_loadInstance($id) {
		$content = $this->getContent();
		if ($content->getInstanceId() == $id)
			return $content;
		$result = null;
		return $result;
	}

	function _node_onchange_child() {
		$this->_questionnaires = null;
		$this->_testChapters = null;
		$this->_chapters = null;
		$this->_files = null;
		parent::_node_onchange_child();
	}
}
