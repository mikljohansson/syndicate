<?php

/**
 * Expression node
 *
 * Use SyndExpression to build an expression tree from a tokenized
 * infix stream. Will use the readExpression(stream) method on the
 * supplied implementation object (IndexQuery) to instanciate the 
 * correct expression object for each token.
 *
 * @access		public
 * @package		synd.core.index
 */
class SyndExpression {
	/**
	 * Height of this expression/branch.
	 *
	 * Used by the indexes to determine number of self-joins of the
	 * term index are needed to satisfy the query.
	 *
	 * @return	integer
	 */
	function getHeight() {
		return 0;
	}

	/**
	 * Check if this expression has precedence over another expression.
	 * @param	SyndExpression	The expression to check precedence over
	 * @return	bool
	 */
	function hasPrecedence($expression) {
		return null == $expression || $this->getPrecedence() >= $expression->getPrecedence();
	}

	/**
	 * Operator precedence used by the precedence parser
	 * return integer
	 */
	function getPrecedence() {
		return 0;
	}

	function toString() {
		return '';
	}

	function isLeftAssociative() {
		return false;
	}

	function isRightAssociative() {
		return false;
	}

    /**
     * Consume operands from stack and push self
     * @param	array	Parser context
     */
    function reduceExpression(&$context) {
		$context['operands'][] = $this;
	}

	/**
	 * Returns the next parser state read from token stream
	 * @param	IndexQuery	The query being parsed
	 * @param	array	Input token stream
	 * @param	array	Parser context
	 * @return	SyndExpression
	 */
	function nextExpression($query, &$stream, &$context) {
		// Allow operators with higher precedence to reduce themselves
		while (null != ($end = end($context['operators'])) && $end->hasPrecedence($this)) {
			array_pop($context['operators']);
			$end->reduceExpression($context);
		}
		
		// Read expression from stream and check for EOF
		if (null !== ($next = $query->readExpression($stream))) {
			// Incompatible operators
			if ($this->isRightAssociative() && $next->isLeftAssociative()) {
				if ($this->hasPrecedence($next))
					return $this;
				return $next;
			}
			
			// Return implicit operator
			if (!$this->isRightAssociative() && !$next->isLeftAssociative()) {
				if (false === prev($stream))
					end($stream);
				$next = $query->defaultExpression();
			}
		}

		// Push this onto operator stack
		$context['operators'][] = $this;
		return $next;
	}

	/**
	 * Visitor pattern
	 * @param	IndexExpressionVisitor
	 */
	function acceptVisitor($visitor) {}

	/**
	 * Creates a balanced expression
	 * 
	 * In a balanced expression all conditional branches are of 
	 * the same height meaning that all the term tables will be in 
	 * use for every conditional branch.
	 *
	 * This is important or a cartesian product would result from 
	 * the dangling tables.
	 *
	 * @return	SyndExpression
	 */
	function getBalanced() {
		return $this->_getBalanced($this->getHeight());
	}

	/**
	 * Implements the actual balancing
	 * @access	protected
	 * @param	integer	Height to balance to
	 * @return	SyndExpression
	 */
	function _getBalanced($height) {
		return $this;
	}
	
	/**
	 * Is this expression bounded or open ended.
	 *
	 * A positive assertion such as a term is bounded while an 
	 * exclude query on its own is unbounded. OR expression needs
	 * both branches to be bounded, AND expression needs only one
	 * branch to be bounded.
	 *
	 * @return	bool
	 */
	function isBounded() {
		return false;
	}
}

/**
 * Expression visitor interface
 * @access		public
 * @package		synd.core.index
 */
interface IndexExpressionVisitor {
	/**
	 * Operand visitor callback (for example terms)
	 * @param	SyndOperator
	 */
	function visitOperand($operand);
	
	/**
	 * Unary operator visitor callback (NOT)
	 * @param	SyndOperator
	 * @param	SyndExpression
	 */
	function visitUnaryOperator($operator, $operand);
	
	/**
	 * Binary operator visitor callback (AND, OR, ...)
	 * @param	SyndOperator
	 * @param	SyndExpression
	 * @param	SyndExpression
	 */
	function visitBinaryOperator($operator, $lhs, $rhs);
}

/**
 * Operand
 */
class SyndOperand extends SyndExpression {
	function getPrecedence() {
		return 50;
	}

	function isBounded() {
		return true;
	}

	function _getBalanced($height) {
		if ($this->getHeight() >= $height) 
			$balanced = parent::_getBalanced($height);
		else {
			$and = new SyndBooleanAND(parent::_getBalanced($height), new SyndNullQuery());
			$balanced = $and->_getBalanced($height);
		}
		return $balanced;
	}

	function acceptVisitor($visitor) {
		$visitor->visitOperand($this);
	}
}

/**
 * Unary operator
 */
class SyndUnaryOperator extends SyndExpression {
	var $_rhs = null;
	
	function SyndUnaryOperator($rhs) {
		$this->_rhs = $rhs;
	}

	function getHeight() {
		return $this->_rhs->getHeight();
	}

	function _getBalanced($height) {
		$clone = parent::_getBalanced($height);
		$clone->_rhs = $this->_rhs->_getBalanced($height);
		return $clone;
	}

	function reduceExpression(&$context) {
     	if (count($context['operands'])) {
     		$this->_rhs = SyndLib::array_pop($context['operands']);
        	parent::reduceExpression($context);
		}
	}
	
	function acceptVisitor($visitor) {
		if (null !== $this->_rhs)
			$visitor->visitUnaryOperator($this, $this->_rhs);
	}
}

/**
 * Binary operator
 */
class SyndBinaryOperator extends SyndExpression {
	var $_lhs = null;
	var $_rhs = null;
	
	function SyndBinaryOperator($lhs, $rhs) {
		$this->_lhs = $lhs;
		$this->_rhs = $rhs;
	}

	function reduceExpression(&$context) {
		if (count($context['operands']) >= 2) {
			$this->_rhs = SyndLib::array_pop($context['operands']);
			$this->_lhs = SyndLib::array_pop($context['operands']);
			$context['operands'][] = $this;
		}
	}

	function getHeight() {
		return 1;
	}

	function isLeftAssociative() {
		return true;
	}
	
	function isRightAssociative() {
		return true;
	}

	function acceptVisitor($visitor) {
		if (null !== $this->_lhs && null !== $this->_rhs)
			$visitor->visitBinaryOperator($this, $this->_lhs, $this->_rhs);
		else if (null !== $this->_lhs)
			$this->_lhs->acceptVisitor($visitor);
		else if (null !== $this->_rhs)
			$this->_rhs->acceptVisitor($visitor);
	}
}

class SyndBooleanOR extends SyndBinaryOperator {
	function SyndBooleanOR($lhs = null, $rhs = null) {
		parent::SyndBinaryOperator($lhs, $rhs, true);
	}

	function getPrecedence() {
		return 10;
	}

	function getHeight() {
		return max($this->_lhs->getHeight(), $this->_rhs->getHeight());
	}

	function isBounded() {
		return null != $this->_lhs && $this->_lhs->isBounded() && 
			null != $this->_rhs && $this->_rhs->isBounded();
	}

	function _getBalanced($height) {
		$clone = parent::_getBalanced($height);
		$clone->_lhs = $this->_lhs->_getBalanced($height);
		$clone->_rhs = $this->_rhs->_getBalanced($height);
		return $clone;
	}
}

class SyndBooleanAND extends SyndBinaryOperator {
	function SyndBooleanAND($lhs = null, $rhs = null) {
		parent::SyndBinaryOperator($lhs, $rhs, false);
	}
	
	function getPrecedence() {
		return 20;
	}

	function getHeight() {
		$height = 0;
		if (null !== $this->_lhs)
			$height += $this->_lhs->getHeight();
		if (null !== $this->_rhs)
			$height += $this->_rhs->getHeight();
		return $height;
	}

	function isBounded() {
		return null != $this->_lhs && $this->_lhs->isBounded() || 
			null != $this->_rhs && $this->_rhs->isBounded();
	}

	function _getBalanced($height) {
		$clone = parent::_getBalanced($height);
		if (null !== ($clone->_lhs = $this->_lhs))
			$height -= $this->_lhs->getHeight();
		$clone->_rhs = $this->_rhs->_getBalanced($height);
		return $clone;
	}
}

class SyndBooleanInclude extends SyndBooleanAND {
	var $_name = 'include';

	function SyndBooleanInclude($lhs = null, $rhs = null) {
		parent::SyndBooleanAND($lhs, $rhs);
	}

	function getPrecedence() {
		return null == $this->_rhs ? 30 : 5;
	}

	function isRightAssociative() {
		return true;
	}

	function reduceExpression(&$context) {
		if (null == $this->_rhs) {
			if (isset($context[$this->_name]))
				$context[$this->_name]->_rhs = $this->_unionOperand($context);
			else {
				$this->_rhs = SyndLib::array_pop($context['operands']);
				$context[$this->_name] = $this;
				$context['operators'][count($context['operators']) ?  min(array_keys($context['operators']))-1 : 0] = $this;
				ksort($context['operators']);
			}
		}
		else {
			$this->_lhs = $this->_leftOperand($context);
			$context['operands'][] = $this;
		}
	}
	
	function _leftOperand(&$context) {
		if (!count($context['operands']))
			return null;
		return new SyndBooleanOR(
			SyndLib::array_pop($context['operands']),
			new SyndNullQuery());
	}
	
	function _unionOperand(&$context) {
		return new SyndBooleanAND(
			$context[$this->_name]->_rhs, 
			SyndLib::array_pop($context['operands']));
	}
}

class SyndBooleanExclude extends SyndBooleanInclude {
	var $_name = 'not';

	function SyndBooleanExclude($lhs = null, $rhs = null) {
		parent::SyndBooleanInclude($lhs, $rhs);
	}

	function isBounded() {
		return null != $this->_lhs && $this->_lhs->isBounded();
	}

	/**
	 * The exclusion will be done in a subquery, so only the LHS adds to the height
	 */
	function getHeight() {
		return $this->_lhs->getHeight();
	}

	function _leftOperand(&$context) {
		return SyndLib::array_pop($context['operands']);
	}

	function _unionOperand(&$context) {
		return new SyndBooleanOR(
			$context[$this->_name]->_rhs, 
			SyndLib::array_pop($context['operands']));
	}

	function _getBalanced($height) {
		$clone = $this;
		if (null != $this->_lhs)
			$clone->_lhs = $this->_lhs->_getBalanced($height);
		return $clone;
	}
}

/**
 * Term query
 */
class SyndTermQuery extends SyndOperand {
	var $_term = null;

	function SyndTermQuery($term) {
		$this->_term = $term;
	}
	
	function getHeight() {
		return 1;
	}

	function toString() {
		return $this->_term;
	}
	
	function setString($string) {
		$this->_term = $string;
	}
}

/**
 * Null term query (for expression balancing)
 */
class SyndNullQuery extends SyndOperand {
	function getHeight() {
		return 1;
	}
}

/**
 * Expression wrapper (for paranthesized subexpressions)
 */
class SyndExpressionWrapper extends SyndOperand {
	var $_expression = null;

	function SyndExpressionWrapper($expression) {
		$this->_expression = $expression;
	}
	
	function acceptVisitor($visitor) {
		$this->_expression->acceptVisitor($visitor);
	}
	
	function getHeight() {
		return $this->_expression->getHeight();
	}

	function isBounded() {
		return $this->_expression->isBounded();
	}
}
