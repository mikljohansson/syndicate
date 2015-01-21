<?php
require_once 'core/model/node/question.class.inc';

class synd_node_multiple_choice extends synd_node_question {
	/**
	 * Cached options
	 * @access	private
	 */
	var $_options = null;
	
	/**
	 * Buffered total answer counts
	 * @access	private
	 */
	var $_totals = null;

	/**
	 * Buffered option answer counts
	 * @access	private
	 */
	var $_counts = null;

	function __factory() {
		parent::__factory();
		$this->data['INFO_LAYOUT'] = 'radio';
	}
	
	function __sleep() {
		$vars = parent::__sleep();
		unset($vars[array_search('_totals', $vars)]);
		unset($vars[array_search('_counts', $vars)]);
		return $vars;
	}
	
	function getName($attempt) {
		return parent::getName($attempt).'[]';
	}
	
	function getCompositeData() {
		$data = parent::getCompositeData();
		$data['options'] = implode("\r\n", 
			SyndLib::array_collect($this->getOptions(), 'INFO_OPTION'));
		return $data;
	}

	function hasInlineOptions() {
		return in_array($this->data['INFO_LAYOUT'],
			array('inline_checkbox', 'inline_radio'));
	}

	function getOptions() {
		if (null !== $this->_options) 
			return $this->_options;
		
		$sql = "
			SELECT o.* FROM synd_question_option o
			WHERE o.question_node_id = '$this->nodeId'
			ORDER BY o.info_weight";

		$this->_options = $this->_db->getAll($sql);
		$this->_node_onchange_internal();
		return $this->_options;
	}
	
	function getOption($id) {
		foreach ($this->getOptions() as $option) {
			if ($id == $option['OPTION_NODE_ID'])
				return $option;
		}
		return null;
	}
	
	function isCorrect($answer) {
		return $answer == $this->getCorrectOptions();
	}
	
	function isCorrectOption($answer) {
		return in_array($answer, $this->getCorrectOptions());
	}

	function getCorrectOptions() {
		$options = array();
		$correct = preg_split('/\s*\n\s*/s', trim($this->data['INFO_CORRECT_ANSWER']));

		foreach ($this->getOptions() as $option) {
			if (in_array($option['INFO_OPTION'], $correct))
				$options[] = $option['OPTION_NODE_ID'];
		}

		return $options;
	}
	
	function getAnswer($attempt) {
		$sql = "
			SELECT a.option_node_id 
			FROM synd_question_option o, synd_answer_option a
			WHERE 
				o.question_node_id = '$this->nodeId' AND
				o.option_node_id = a.option_node_id AND
				a.attempt_node_id = '$attempt->nodeId'";
		return $this->_db->getCol($sql, 0);	
	}

	function addAnswer($attempt, $answer) {
		if (is_array($answer)) {
			foreach (array_filter($answer) as $option) {
				$option = $this->_db->quote($option);
				$sql = "
					INSERT INTO synd_answer_option
					(attempt_node_id, option_node_id)
					VALUES
					('$attempt->nodeId', $option)";
				$this->_db->query($sql);
			}
		}
	}
	
	function getAnswerCount() {
		$parent = $this->getParent();
		$filters = $parent->getStatisticsFilters();
		$hash = SyndLib::hash($filters);

		if (!isset($this->_totals[$hash])) {
			$sql = " 
				SELECT COUNT(DISTINCT s.attempt_node_id) 
				FROM synd_question_option o, synd_answer_option s
				WHERE 
					o.question_node_id = '$this->nodeId' AND
					o.option_node_id = s.option_node_id".$this->_filter($filters);
			$this->_totals[$hash] = $this->_db->getOne($sql);
		}

		return $this->_totals[$hash];
	}
	
	function getOptionAnswerCount($option) {
		$parent = $this->getParent();
		$filters = $parent->getStatisticsFilters();
		$hash = SyndLib::hash($filters);
		
		if (!isset($this->_counts[$hash])) {
			$sql = "
				SELECT o.option_node_id, COUNT(*) 
				FROM synd_question_option o, synd_answer_option s
				WHERE 
					o.question_node_id = '$this->nodeId' AND
					o.option_node_id = s.option_node_id".$this->_filter($filters)."
				GROUP BY o.option_node_id";
			$this->_counts[$hash] = $this->_db->getAssoc($sql);
		}

		return isset($this->_counts[$hash][$option]) ? $this->_counts[$hash][$option] : 0;
	}

	function _filter($filters, $alias = 's') {
		if (empty($filters))
			return null;
		
		$sql = null;
		foreach ($filters as $filter) {
			$sql .= " AND
				$alias.attempt_node_id IN ($filter)";
		}
		
		return $sql;
	}
	
	function getAttemptFilter($options) {
		if (empty($options))
			return null;
	
		$sql = "
			SELECT s0.attempt_node_id 
			FROM ";
		for ($i=0; $i<count($options); $i++) 
			$sql .= "synd_answer_option s$i, ";
		$sql = rtrim($sql, ", ");
		
		$i = 0;
		$sql .= "
			WHERE ";
		foreach ($options as $option) {
			if (0 != $i) 
				$sql .= " AND s$i.attempt_node_id = s0.attempt_node_id AND ";
			
			$sql .= "s$i.option_node_id = ".$this->_db->quote($option);
			$i++;
		}
		
		return $sql;
	}
	
	function getOptionAnswerFrequency($option) {
		if (0 != ($count = $this->getOptionAnswerCount($option)))
			return $count / $this->getAnswerCount();
		return 0;
	}
	
	function merge($data) {
		parent::merge($data);

		if ($this->hasInlineOptions()) {
			if (isset($data['INFO_QUESTION'])) {
				preg_match_all('/<\(?(\w+)\)(.*?)>/', $data['INFO_QUESTION'], $matches);
				$options = $matches[1];
			}
		}
		else if (isset($data['options']))
			$options = preg_split("/\s*\n\s*/s", trim($data['options']));
		
		if (isset($options)) {
			$oldOptions = $this->getOptions();
			$newOptions = array();
			$weight = 0;

			foreach ($options as $text) {
				// Check for old option (keep the old option_node_id and answers)
				$oldFound = false;
				foreach ($oldOptions as $key => $option) {
					if ($option['INFO_OPTION'] == $text) {
						$option['INFO_WEIGHT'] = $weight++;
						$newOptions[] = $option;
						unset($oldOptions[$key]);
						$oldFound = true;
						break;
					}
				}
				
				// Add new option
				if (!$oldFound) {
					$newOptions[] = array(
						'OPTION_NODE_ID' => 'option.'.$this->_db->nextId('synd_question_option'),
						'QUESTION_NODE_ID' => $this->nodeId,
						'INFO_OPTION' => $text,
						'INFO_WEIGHT' => $weight++);
				}
			}
			
			$this->_options = $newOptions;
		}
	}
	
	function __clone() {
		$options = $this->getOptions();
		parent::__clone();
		foreach (array_keys($options) as $key) {
			$options[$key]['OPTION_NODE_ID'] = 'option.'.$this->_db->nextId('synd_question_option');
			$options[$key]['QUESTION_NODE_ID'] = $this->nodeId;
		}
		$this->_options = $options;
	}
	
	function _entity_onpostflush() {
		require_once 'core/db/SyndDBLib.class.inc';
		parent::_entity_onpostflush();
		
		if (null !== $this->_options) {
			$oldOptions = array_flip($this->_db->getCol("
				SELECT o.option_node_id FROM synd_question_option o
				WHERE o.question_node_id = '$this->nodeId'", 0));

			foreach ($this->_options as $option) {
				if (isset($oldOptions[$option['OPTION_NODE_ID']])) {
					unset($oldOptions[$option['OPTION_NODE_ID']]);
					$sql = "
						UPDATE synd_question_option
						SET ".SyndDBLib::sqlUpdate($this->_db->quote($option),false)."
						WHERE option_node_id = ".$this->_db->quote($option['OPTION_NODE_ID']);
					$this->_db->query($sql);
				}
				else {
					$sql = "
						INSERT INTO synd_question_option
						(".implode(', ', array_keys($option)).")
						VALUES
						(".implode(', ', $this->_db->quote($option)).")";
					$this->_db->query($sql);
				}
			}
			
			// Delete obsolete options
			if (count($oldOptions)) {
				$sql = "
					DELETE FROM synd_question_option
					WHERE option_node_id IN (".
					implode(', ',$this->_db->quote(array_keys($oldOptions))).")";
				$this->_db->query($sql);
			}
			
			$this->_options = null;
			$this->_counts = null;
		}
	}
	
	function _view_addStatisticsFilter(Request $request, Template $page) {
		if (!$this->isPermitted('statistics'))
			throw new ForbiddenException();
		if (!isset($request[2]))
			return null;
			
		$parent = $this->getParent();
		$parent->addStatisticsFilter($this, $request[2]);
	}

	function _view_delStatisticsFilter(Request $request, Template $page) {
		if (!$this->isPermitted('statistics'))
			throw new ForbiddenException();
		if (!isset($request[2]))
			return null;
			
		$parent = $this->getParent();
		$parent->delStatisticsFilter($this, $request[2]);
	}
	
	function _view_xls(Request $request, Template $page) {
		if (!$this->isPermitted('statistics'))
			throw new ForbiddenException();

		$parent = $this->getParent();
		$filters = $parent->getStatisticsFilters();

		// Retrieve counts per column and option
		$sql = "
			SELECT 
				ao1.option_node_id COLUMN_NODE_ID, 
				ao2.option_node_id ROW_NODE_ID, 
				COUNT(*) CNT
			FROM 
				 synd_question_option qo1, 
				 synd_answer_option ao1, synd_answer_option ao2
			WHERE
				 qo1.question_node_id = ".$this->_db->quote($this->nodeId)." AND
				 qo1.option_node_id = ao1.option_node_id AND
				 ao1.attempt_node_id = ao2.attempt_node_id ".$this->_filter($filters,'ao1')."
			GROUP BY ao1.option_node_id, ao2.option_node_id";

		$perOption = array();
		foreach ($this->_db->getAll($sql) as $row)
			$perOption[$row['COLUMN_NODE_ID']][$row['ROW_NODE_ID']] = $row['CNT'];

		// Retrieve counts per column and question
		$sql = "
			SELECT 
				ao1.option_node_id COLUMN_NODE_ID, 
				qo2.question_node_id QUESTION_NODE_ID, 
				COUNT(DISTINCT ao1.attempt_node_id) CNT
			FROM 
				 synd_question_option qo1, synd_question_option qo2, 
				 synd_answer_option ao1, synd_answer_option ao2
			WHERE
				 qo1.question_node_id = ".$this->_db->quote($this->nodeId)." AND
				 qo1.option_node_id = ao1.option_node_id AND
				 qo2.option_node_id = ao2.option_node_id AND
				 ao1.attempt_node_id = ao2.attempt_node_id ".$this->_filter($filters,'ao1')."
			GROUP BY ao1.option_node_id, qo2.question_node_id";

		$perQuestion = array();
		foreach ($this->_db->getAll($sql) as $row)
			$perQuestion[$row['COLUMN_NODE_ID']][$row['QUESTION_NODE_ID']] = $row['CNT'];

		// Retrive counts per column
		$sql = "
			SELECT ao1.option_node_id COLUMN_NODE_ID, COUNT(*) CNT
			FROM synd_question_option qo1, synd_answer_option ao1
			WHERE
				 qo1.question_node_id = ".$this->_db->quote($this->nodeId)." AND
				 qo1.option_node_id = ao1.option_node_id ".$this->_filter($filters,'ao1')."
			GROUP BY ao1.option_node_id";
		$perColumn = $this->_db->getAssoc($sql);

		$content = $page->fetchnode($this, 'report/grouped.xls.tpl', array(
			'perOption' 	=> $perOption,
			'perQuestion' 	=> $perQuestion,
			'perColumn' 	=> $perColumn,
			));
		
		return array('page' => null, 'content' => $content);
	}
}
