<?php
require_once 'core/db/query/sql.class.inc';
require_once 'core/index/SyndExpression.class.inc';

/**
 * Transforms a query expression into an sql query
 *
 * Used by the term index and it's assorted extensions to 
 * transform a SyndExpression tree into an sql query.
 *
 * @package		synd.core.index
 */
class SqlQueryBuilder extends synd_query_sql implements IndexExpressionVisitor {
	/**
	 * @access	protected
	 */
	var $_index = null;

	/**
	 * Temp to hold the filter string while rendering an expression
	 * @var		string
	 * @access	protected
	 */
	var $_sql = null;
	
	/**
	 * @param	AbstractIndex
	 */
	function __construct($index, $prefix = null) {
		parent::__construct($index->_db, $prefix);
		$this->_index = $index;
	}
	
	function filter($expression) {
		$this->where($this->parseExpression($expression));
	}
	
	function parseExpression($expression) {
		$this->_sql = null;
		$expression->acceptVisitor($this);
		return $this->_sql;
	}
	
	function enterExpression() {
		$this->_sql .= '(';
	}
	
	function leaveExpression() {
		$this->_sql .= ')';
	}
	
	function visitOperand($operand) {
		if (($operand instanceof SyndTermQuery))
			$this->_sql .= $this->_index->runHook('query_visit_term', array($this, $operand));
		else if (($operand instanceof SyndNullQuery))
			$this->_sql .= $this->_index->runHook('query_visit_null', array($this, $operand));
	}

	function visitUnaryOperator($operator, $operand) {
		if (null !== ($filter = $this->_index->runHook('query_visit_operator', 
			array($this, $operator, $operand))))
			$this->_sql .= null != $filter ? '('.$filter.')' : $filter;
		else
			$operand->acceptVisitor($this);
	}
	
	function visitBinaryOperator($operator, $lhs, $rhs) {
		if (null !== ($filter = $this->_index->runHook('query_visit_operator', 
			array($this, $operator, $lhs, $rhs))))
			$this->_sql .= null != $filter ? '('.$filter.')' : $filter;
		else if (($operator instanceof SyndBooleanExclude)) {
			$lhs->acceptVisitor($this);
			$this->_exclude($rhs);
		}
		else if (($operator instanceof SyndBooleanAND))
			$this->_and($lhs, $rhs);
		else if (($operator instanceof SyndBooleanOR))
			$this->_or($lhs, $rhs);
	}
	
	function _and($lhs, $rhs) {
		$this->enterExpression();

		$lhs->acceptVisitor($this);
		$this->_sql .= ' AND ';
		$rhs->acceptVisitor($this);

		$this->leaveExpression();
	}
	
	function _or($lhs, $rhs) {
		$this->enterExpression();

		// Build left hand side in separate table allocation context
		$this->pushContext();
		$lhs->acceptVisitor($this);
		$this->popContext();
		
		$this->_sql .= ' OR ';

		$rhs->acceptVisitor($this);

		$this->leaveExpression();
	}
	
	/**
	 * Compiles a subquery to determine which documents match the 
	 * NOT'ed expression and then excludes them from the main query
	 * via 'docid NOT IN(..)'
	 */
	function _exclude($rhs) {
		$builder = new SqlQueryBuilder($this->_index, 's');
		
		// Compile virtual table
		$sd = $builder->join('synd_search_document','d',array('docid'));
		$builder->filter($rhs->getBalanced());
		
		$sql = "
				SELECT $sd.docid FROM ".$builder->getFrom()."
				WHERE ".$builder->getWhere();
		
		// Limit to 25k exclusion
		$sql = $this->_index->_db->limit($sql, 0, 25000);

		// Left join with the outer document table
		$d = $this->join('synd_search_document','d',array('docid'));
		$this->_from[$d] = $this->_from[$d]."
			LEFT JOIN ($sql) t$sd 
			ON (t$sd.docid = $d.docid)";
			
		$this->where("t$sd.docid IS NULL");
	}
}
