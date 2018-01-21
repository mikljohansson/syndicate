<?php
require_once 'core/model/node/question.class.inc';

class synd_node_open_ended_text extends synd_node_question {
	var $_counts = array();
	
	function __factory() {
		parent::__factory();
		$this->data['INFO_LAYOUT'] = 'input';
	}
	
	function addAnswer($attempt, $answer) {
		$infoAnswer = null;
		if (null != trim($answer))
			$infoAnswer = $this->_db->wrapBinary(serialize(trim($answer)));
		
		$sql = "
			INSERT INTO synd_answer_text
			(attempt_node_id, question_node_id, info_answer)
			VALUES
			('$attempt->nodeId', '$this->nodeId', 
			 ".$this->_db->quote($infoAnswer).")";
		$this->_db->query($sql);

		$this->_counts = array();
		$this->_node_onchange_internal();
	}

	function getAnswer($attempt) {
		$sql = "
			SELECT a.info_answer FROM synd_answer_text a
			WHERE 
				a.attempt_node_id = '$attempt->nodeId' AND
				a.question_node_id = '$this->nodeId'";
		if (null != ($answer = $this->_db->getOne($sql)))
			return unserialize($this->_db->unwrapBinary($answer));
		return null;
	}

	function getAnswers() {
		$parent = $this->getParent();
		$filters = $parent->getStatisticsFilters();
		$answers = array();

		$sql = "
			SELECT s.* FROM synd_answer_text s
			WHERE 
				s.question_node_id = '$this->nodeId' AND
				s.info_answer IS NOT NULL".$this->_filter($filters);
		foreach ($this->_db->getAll($sql) as $answer) {
			if (null != ($answer['INFO_ANSWER'] = unserialize($this->_db->unwrapBinary($answer['INFO_ANSWER']))))
				$answers[] = $answer;
		}
		
		// Preload attempts
		SyndNodeLib::getInstances(SyndLib::array_collect($answers, 'ATTEMPT_NODE_ID'));

		return $answers;
	}

	function getAnswerCount() {
		$parent = $this->getParent();
		$filters = $parent->getStatisticsFilters();
		$hash = SyndLib::hash($filters);

		if (!isset($this->_counts[$hash])) {
			$sql = "
				SELECT COUNT(*) FROM synd_answer_text s
				WHERE 
					s.question_node_id = '$this->nodeId' AND
					s.info_answer IS NOT NULL".$this->_filter($filters);
			$this->_counts[$hash] = $this->_db->getOne($sql);
			$this->_node_onchange_internal();
		}
		
		return $this->_counts[$hash];
	}

	function _filter($filters) {
		if (empty($filters))
			return null;
		
		$sql = null;
		foreach ($filters as $filter) {
			$sql .= " AND
				s.attempt_node_id IN ($filter)";
		}
		
		return $sql;
	}
	
	function isCorrect($answer) {
		$correct = $this->stripAnswerText($this->data['INFO_CORRECT_ANSWER']);
		$answer = $this->stripAnswerText($answer);
		return preg_match($this->answerExpression($correct), $answer);
	}

	/**
	 * Transforms an answer into a regexp.
	 *
	 * Conditional expression
	 *  '<foo/bar>' becomes '(foo|bar)'
	 *
	 * @param	string	The answer
	 * @return	string
	 */
	function answerExpression($answer) {
		static $lambda = null;
		if (null == $lambda)
			$lambda = create_function('$m', 'return "\\s*(".preg_replace("/\s*\/\s*/","|",trim($m[1])).")\\s*";');
		
	 	// '<foo/bar>' becomes '(foo|bar)'
		$answer = preg_quote($answer, '/');
		$answer = preg_replace_callback('/\s*\\\<(([^<\/>]+\/)+[^<\/>]*)\\\>\s*/', $lambda, $answer);
		$answer = str_replace('/','\\/',$answer);
		
		return "/^{$answer}$/";
	}

	function stripAnswerText($text) {
		$text = preg_replace('/([^\w\-\'<>\/]+| {2,})/s', ' ', $text);
		if (!$this->isCaseSensitive())
			$text = strtolower($text);
		return trim($text);
	}
	
	function _view_xls(Request $request, Template $page) {
		if (!$this->isPermitted('statistics'))
			throw new ForbiddenException();
		return array('content' => $page->fetchnode($this,'xls.tpl'), 'page' => null);
	}
}

