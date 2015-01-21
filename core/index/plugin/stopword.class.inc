<?php
require_once 'core/index/SyndIndexExtension.class.inc';
require_once 'core/index/SyndExpression.class.inc';

/**
 * Stopword filter extension
 */
class synd_plugin_stopword extends SyndIndexExtension {
	function initialize($index) {
		parent::initialize($index);
		if ($index->getDocumentCount())
			$index->attachHook('expression', array($this, '_callback_expression'), 20);
	}
	
	function _callback_expression(&$result, $expression) {
		$statistics = new StatisticsExpressionVisitor($this->_index);
		$expression->acceptVisitor($statistics);
		if (!count($terms = $statistics->getTerms()))
			return;
			
		// Retrieve term frequencies
		$words = array();
		foreach (array_unique(SyndLib::invoke($terms, 'toString')) as $word)
			$words[] = $this->_index->termProcess($word);
		
		$sql = "
			SELECT td.term, td.n FROM synd_search_term td
			WHERE td.term IN (".implode(',',$this->_index->_db->quote($words)).")";
		$freqs = $this->_index->_db->getAssoc($sql);
		
		// Prune stopwords from query
		$expression->acceptVisitor(new StopwordFilteringVisitor($this, $freqs));
	}
}

/**
 * Stopword pruning visitor
 * @package	synd.core.index
 */
class StopwordFilteringVisitor implements IndexExpressionVisitor {
	var $_extension = null;
	var $_freqs = null;
	var $_limit = 0.35;
	
	function __construct($extension, $freqs) {
		$this->_extension = $extension;
		$this->_freqs = $freqs;
	}
	
	function visitOperand($operand) {}
	function visitUnaryOperator($operator, $operand) {}
	
	function visitBinaryOperator($operator, $lhs, $rhs) {
		if (($operator instanceof SyndBooleanOR)) {
			if (($lhs instanceof SyndTermQuery)) {
				$word = $this->_extension->_index->termProcess($lhs->toString());
				if (isset($this->_freqs[$word]) && $this->_freqs[$word] / $this->_extension->_index->getDocumentCount() >= $this->_limit)
					$expression->_lhs = new SyndNullQuery();
			}

			if (($rhs instanceof SyndTermQuery)) {
				$word = $this->_extension->_index->termProcess($rhs->toString());
				if (isset($this->_freqs[$word]) && $this->_freqs[$word] / $this->_extension->_index->getDocumentCount() >= $this->_limit)
					$expression->_rhs = new SyndNullQuery();
			}
		}
	}
}
