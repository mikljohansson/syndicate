<?php
require_once 'core/model/DatabaseEntity.class.inc';

/**
 * @todo	getQuestionNumber() that looks at the position of this question in the parents' children 
 */
class synd_node_question extends AbstractDatabaseEntity {
	function __factory() {
		$this->data['NODE_ID'] = $this->classId().'.'.$this->_db->nextId('synd_question');
		parent::__factory();
	}

	function __clone() {
		$this->data['NODE_ID'] = $this->classId().'.'.$this->_db->nextId('synd_question');
		parent::__clone();
	}

	static function getEntityQuery(DomainStorage $storage) {
		return self::createQuery($storage, 'synd_question');
	}

	function nodeId() {
		return $this->data['NODE_ID'];
	}

	function hasPermission($user, $perm, $arg = null) {
		if ('write' == $perm && $this->isNew() && !$user->isNull())
			return true;
		$parent = $this->getParent();
		if ('read' == $perm && 'full_view_answers.tpl' == $arg && $parent->hasPermission($user, 'statistics'))
			return true;
		if ('merge' == $perm && in_array($arg, array(
			'PARENT_NODE_ID','INFO_LAYOUT','INFO_QUESTION','INFO_CORRECT_ANSWER','INFO_CORRECT_EXPLANATION',
			'INFO_INCORRECT_EXPLANATION','FLAG_PROGRESS','FLAG_DIAGNOSTIC','FLAG_CASE_SENSITIVE')))
			return $this->hasPermission($user, 'write');
		return parent::hasPermission($user, $perm, $arg);
	}

	function toString() {
		return $this->data['INFO_QUESTION'];
	}

	function getWeight() {
		return $this->data['INFO_WEIGHT'];
	}
	
	function setWeight($weight) {
		$this->data['INFO_WEIGHT'] = $weight;
		$this->_save();
	}
	
	function getLayout() {
		return $this->data['INFO_LAYOUT'];
	}

	function getIndexDocument($index) {
		return $index->createComposite(array(
			$index->createFragment($this->toString()),
			$index->createFragment($this->data['INFO_CORRECT_EXPLANATION'], null, 0.6),
			$index->createFragment($this->data['INFO_INCORRECT_EXPLANATION'], null, 0.6),
			));
	}
	
	function setParent($parent) {
		$this->data['PARENT_NODE_ID'] = $parent->nodeId;
	}

	function getName($attempt) {
		return $attempt->getName()."[$this->nodeId]";
	}

	function getCourse() {
		$parent = $this->getParent();
		if (!$parent->isNull())
			return $parent->getCourse();
		return SyndNodeLib::getInstance('null.null');
	}
	
	function getBranch() {
		$parent = $this->getParent();
		if (!$parent->isNull())
			return $parent->getBranch();
		return array();
	}

	function getHandler() {
		$parent = $this->getParent();
		return $parent->getHandler();
	}

	function getParent() {
		if (null === ($parent = SyndNodeLib::getInstance($this->data['PARENT_NODE_ID'])))
			$parent = parent::getParent();
		return $parent;
	}

	function getPage() {
		$parent = $this->getParent();
		return $parent->getPage();
	}

	function getQuestion() {
		return $this->data['INFO_QUESTION'];
	}

	function getExcelFilename($number = null) {
		$name = null;
		if (null != $number)
			$name .= SyndLib::translate('Question %d', $number);
		if (null != ($text = preg_replace('/[^\wедц ]/i', '', trim(tpl_chop($this->toString(),25)))))
			$name .= ' - '.$text;
		if (null != $name)
			return trim($name,'- ').'.xls';
		return "{$this->nodeId}.xls";
	}
	
	/**
	 * Add an answer to a question.
	 * @abstract
	 * @param	synd_node_attempt
	 * @param	string	Submitted answer
	 */
	function addAnswer($attempt, $answer) {}

	/**
	 * Returns a previously saved answer
	 * @abstract
	 * @param	synd_node_attempt
	 * @return	mixed
	 */
	function getAnswer($attempt) {}

	/**
	 * Returns all answer for this question
	 * @abstract
	 */
	function getAnswers() {}

	/**
	 * Returns the number of answer to this question
	 * @abstract
	 */
	function getAnswerCount() {}

	/**
	 * Checks if an answer is correct
	 * @abstract
	 */
	function isCorrect($answer) {}
	
	function getCorrectAnswer() {
		return synd_htmlspecialchars(strtr($this->data['INFO_CORRECT_ANSWER'], '<>', '  '));
	}
	
	function isProgressQuestion() {
		return $this->data['FLAG_PROGRESS'];
	}
	
	function isDiagnosticQuestion() {
		return $this->data['FLAG_DIAGNOSTIC'];
	}

	function isCaseSensitive() {
		return $this->data['FLAG_CASE_SENSITIVE'];
	}

	function merge($data) {
		$data['FLAG_PROGRESS'] = (int)!empty($data['FLAG_PROGRESS']);
		$data['FLAG_DIAGNOSTIC'] = (int)!empty($data['FLAG_DIAGNOSTIC']);
		$data['FLAG_CASE_SENSITIVE'] = (int)!empty($data['FLAG_CASE_SENSITIVE']);

		if (empty($this->data['PARENT_NODE_ID']) && !empty($data['PARENT_NODE_ID']) && 
			null != ($parent = SyndNodeLib::getInstance($data['PARENT_NODE_ID']))) {
			$parent->insertBefore($this);
		}
		
		parent::merge($data);
	}
	
	function _entity_onpreflush() {
		global $synd_user;
		parent::_entity_onpreflush();
		$this->data['UPDATE_NODE_ID'] = $synd_user->nodeId;
		$this->data['TS_UPDATE'] = time();
	}
}
