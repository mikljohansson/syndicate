<?php
require_once 'core/model/DatabaseEntity.class.inc';

class synd_node_attempt extends AbstractDatabaseEntity {
	var $_progress  = -1;
	var $_sequence  = 0;
	var $_questions = null;

	function __factory() {
		global $synd_user;
		$this->data['NODE_ID'] = $this->classId().'.'.$this->_db->nextId('synd_attempt');
		$this->data['CLIENT_NODE_ID'] = $synd_user->nodeId;
		$this->data['TS_CREATE'] = time();
		parent::__factory();
	}
	
	function __clone() {
		$this->data['NODE_ID'] = $this->classId().'.'.$this->_db->nextId('synd_attempt');
		$this->data['TS_CREATE'] = time();
		parent::__clone();
	}

	static function getEntityQuery(DomainStorage $storage) {
		return self::createQuery($storage, 'synd_attempt');
	}

	function nodeId() {
		return $this->data['NODE_ID'];
	}

	function hasPermission($user, $perm, $arg = null) {
		$client = $this->getCustomer();
		if ('read' == $perm && $client->hasAuthorization($user, $this))
			return true;
		if ('answer' == $perm && $this->isNew() && ($client->isNull() || $client->hasAuthorization($user, $this)))
			return true;
		return parent::hasPermission($user, $perm, $arg);
	}
	
	function setAnonymous() {
		$this->data['CLIENT_NODE_ID'] = null;
		$this->_save();
	}
	
	function toString() {
		$parent = $this->getParent();
		return $parent->toString();
	}

	function getHandler() {
		$parent = $this->getParent();
		return $parent->getHandler();
	}

	function getName() {
		return "mplex[".$this->getHandler()."/invoke/{$this->nodeId}/submit/][answer]";
	}

	function getAnswer($question) {
		return $question->getAnswer($this);
	}
	
	function hasAnswers() {
		return $this->data['INFO_ANSWERS'];
	}

	function getParent() {
		if (null === ($parent = SyndNodeLib::getInstance($this->data['PARENT_NODE_ID'])))
			$parent = parent::getParent();
		return $parent;
	}

	function setParent($parent) {
		parent::setParent($parent);
		$this->data['PARENT_NODE_ID'] = $parent->nodeId;
	}

	function getCustomer() {
		if (null != ($parent = SyndNodeLib::getInstance($this->data['CLIENT_NODE_ID'])))
			return $parent;
		return SyndNodeLib::getInstance('user_null.null');
	}
	
	function getColor() {
		if ($this->getProgress() < 0.75)
			return 'red';
		else if ($this->getProgress() < 0.90)
			return 'yellow';
		else
			return 'green';
	}
	
	function getProgress() {
		if ($this->getAnswerCount())
			return $this->getCorrectCount() / $this->getAnswerCount();
		return null;
	}
	
	static function getAttempts($parent, $client) {
		$storage = SyndNodeLib::getDefaultStorage('attempt');
		$database = $storage->getDatabase();
		$sql = "
			SELECT a.node_id, a.ts_create FROM synd_attempt a
			WHERE 
				a.parent_node_id = ".$database->quote($parent->nodeId)." AND
				a.client_node_id = ".$database->quote($client->nodeId)."
			ORDER BY a.ts_create DESC";
		return $storage->getInstances($database->getCol($sql));
	}
	
	/**
	 * Returns the best attempt given a client
	 *
	 * Returns the best attempt or null if no attempts exist, uses the
	 * cache api to cache the results.
	 *
	 * @param	DomainObject			Parent node
	 * @param	SyndUser			Client to fetch for
	 * @return	synd_node_attempt
	 */
	static function getBestAttempt($parent, $client) {
		static $cache = array();

		if (!isset($cache[$client->nodeId])) {
			$sql = "
				SELECT a.parent_node_id, a.node_id 
				FROM synd_attempt a
				WHERE 
					a.client_node_id = '$client->nodeId' AND
					a.info_answers > 0
				ORDER BY a.info_correct / a.info_answers";
			$cache[$client->nodeId] = $parent->_db->getAssoc($sql);

			// Preload other best attempts for same client
			$parent->_storage->getInstances($cache[$client->nodeId]);
		}

		if (isset($cache[$client->nodeId][$parent->nodeId]))
			return $parent->_storage->getInstance($cache[$client->nodeId][$parent->nodeId]);
		return null;
	}
	
	function addQuestion(synd_node_question $question) {
		$this->_questions[] = $question->nodeId;
	}
	
	function getQuestions() {
		if (null !== $this->_questions)
			return SyndNodeLib::getInstances($this->_questions);
		$sql = "
			SELECT aq.question_node_id FROM synd_attempt_questions aq
			WHERE aq.attempt_node_id = '$this->nodeId'
			ORDER BY aq.info_weight";
		return SyndNodeLib::getInstances($this->_db->getCol($sql,0));
	}
	
	function getAnswerCount() {
		return $this->data['INFO_ANSWERS'];
	}
	
	function getCorrectCount() {
		return $this->data['INFO_CORRECT'];
	}
	
	/**
	 * Called when the user submits his answers.
	 */
	function _view_submit(Request $request, Template $page) {
		static $confirmed = false;
		if (!$this->isPermitted('answer')) {
			if ($this->isPermitted('read')) {
				if ($confirmed)
					return null;
				$confirmed = true;
				return array('content' => $page->fetchnode($this, 'confirm_already_replied.tpl'));
			}
			throw new ForbiddenException();
		}

		// Commit node for the sake of foreign keys
		if ($this->isNew()) {
			$this->save();
			$this->flush();
		}
		
		foreach (array_keys($questions = $this->getQuestions()) as $key) {
			if (isset($request['answer'][$questions[$key]->nodeId])) {
				$answer = $request['answer'][$questions[$key]->nodeId];
				$questions[$key]->addAnswer($this, $answer);
				
				$this->data['INFO_ANSWERS']++;
				if ($questions[$key]->isCorrect($answer))
					$this->data['INFO_CORRECT']++;
				$this->_save();
			}
		}
		
		parent::_node_onchange_child();
	}
	
	function _entity_onpostflush() {
		parent::_entity_onpostflush();
		if (null !== $this->_questions) {
			foreach ($this->_questions as $id) {
				$sql = "
					INSERT INTO synd_attempt_questions
					(attempt_node_id, question_node_id, info_weight)
					VALUES
					('$this->nodeId', '$id', ".($this->_sequence++).")";
				$this->_db->query($sql);
			}
			$this->_questions = null;
		}
	}
}

/**
 * Training attempt case
 * @access	public
 * @package	synd.core.model
 */
class SyndTrainingAttempt {
	var $nodeId = 'train';
	var $_answers = array();

	function SyndTrainingAttempt() {
		if (!empty($_REQUEST['answer']))
			$this->_answers = $_REQUEST['answer'];
	}
	
	function getAnswer($question) {
		if (isset($this->_answers[$question->nodeId]))
			return $this->_answers[$question->nodeId];
		return null;
	}
	
	function hasAnswers() {
		return count($this->_answers);
	}
	
	function getName() {
		return 'answer';
	}
}
