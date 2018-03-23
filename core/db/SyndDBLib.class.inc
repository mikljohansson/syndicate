<?php

/**
 * SQL Utility class
 *
 * @static
 * @access		public
 * @package		synd.core.db
 */
class SyndDBLib {
	/**
	 * Use Database->quote() if possible. 
	 * 
	 * This is only for generic quoting like for ODBC databases etc.
	 *
	 * @param	mixed	Value to quote
	 * @param	string	String to enclose value in
	 * @return	mixed
	 */
	static function quote($str, $enclose = "'") {
		if (is_array($str)) {
			foreach (array_keys($str) as $key)
				$str[$key] = SyndDBLib::quote($str[$key], $enclose);
			return $str;
		}
		else if (!is_float($str))
			return $enclose.str_replace("'", "''", $str).$enclose;
		return $str;
	}

	/** 
	 * Splits a searchstring into an array of arguments.
	 *
	 * Honors "foo bar" (quoted string) and all variants of +/-
	 *
	 * @param	string	String to parse
	 * @return	array
	 */
	static function parseSearchString($string) {
		preg_match_all('/(?:\s+|^)([+-\s]*"?(?(?<=")[^"]+|[^\s]+))/', $string, $matches);
		return preg_replace('/\s*"\s*|\s+$|(?<=[+-])[+-\s]+/', '', $matches[1]);
	}
	
	/**
	 * Transform a numeric boolean expression into an sql statement
	 * @param	string	Expression to transform
	 * @param	string	Column to match against
	 * @return	string
	 */
	static function parseNumeric($expression, $column) {
		$rop = '<>=!';
		$cop = '|&';
		$gop = '()';

		$expression = preg_replace("/[^\d$rop$cop$gop]/", '', $expression);
		if (!preg_match("/[$rop]/", $expression))
			return "$column=$expression";

		$expression = preg_replace("/[$rop]+(\s+[$rop]+)/", '\1', $expression);
		$expression = preg_replace("/[$rop]+(\s+[$cop]+)/", '\1', $expression);
		$expression = preg_replace("/[\d]+(\s+[\d]+)/", '\1', $expression);

		$expression = preg_replace("/([$rop]+\d+)/", "$column\\1", $expression);
		$expression = preg_replace("/(\d+[$rop]+)/", "\\1 $column", $expression);

		$expression = preg_replace('/[|]+/', ' OR ', $expression);
		$expression = preg_replace('/[&]+/', ' AND ', $expression);
		
		return $expression;
	}
	
	/**
	 * Builds a WHERE statement from a search string.
	 *
	 * Honours "foo bar" quotation in searchstring.
	 * Default AND/OR algorithm can be choosen and search terms can be
	 * forced AND by specifying + in front of them or negotiated by using -
	 * 
	 * @param	string	String containing searchterms
	 * @param	array	Columns to search to a match
	 * @param	string	Implicit operator {AND,OR}
	 * @param	integer	Minimum length of search term
	 * @return	string
	 */
	static function sqlLikeExpr($query, $cols, $cmp = 'AND', $minLen = 2, $lowerCase = true) {
		$andI = 0; 
		$andSql = ''; 
		$orI = 0;
		$orSql = ''; 
		$sql = null;
		
		// Extract searchArgs from searchString
		$terms = SyndDBLib::parseSearchString($query);

		// Build WHERE expression from searchArgs
		foreach ($terms as $term) {
			// Check min length of searchArg 
			if (strlen($term) < $minLen || (('+' == $term[0] || '-' == $term[0]) && strlen($term) < $minLen+1))
				continue;
			
			$not   = '';
			$term = str_replace('?','_',str_replace('*','%',$term));
			if ($lowerCase)
				$term = strtolower($term);

			// If OR is used and search term is not forced AND (+foo)
			if ('OR' == $cmp && '+' != $term[0]) {
				// Force NOT (-foo)
				if ('-' == $term[0]) {
					$term = ltrim($term, '-');
					$not   = 'NOT';
				}
				$orSql .= ($orI ? ($not?' AND (':' OR ('):' (');

				// Build expression for all columns
				$j=0;
				foreach ($cols as $col) {
					$orSql .= ($j?($not?' AND ':' OR '):'');
					$orSql .= $lowerCase ? "LOWER(CAST($col AS VARCHAR(255)))" : $col;
					$orSql .= " $not LIKE ".SyndDBLib::quote("%$term%");
					$j++;
				}
				$orSql .= ")\n ";
				$orI++;
			}
			else {	// Assume AND
				$term = ltrim($term, '+');

				// Force NOT (-foo)
				if ('-' == $term[0]) {
					$term = ltrim($term, '-');
					$not   = 'NOT';
				}
				$andSql .= ($andI ? ' AND (':' (');

				// Build expression for all columns
				$j=0;
				foreach ($cols as $col) {
					$andSql .= ($j?($not?' AND ':' OR '):'');
					$andSql .= $lowerCase ? "LOWER(CAST($col AS VARCHAR(255)))" : $col;
					$andSql .= " $not LIKE ".SyndDBLib::quote("%$term%");
					$j++;
				}
				$andSql .= ") \n";
				$andI++;
			}
		}

		if ($andSql || $orSql)
			$sql .= ($orSql?"($orSql)":'').($andSql && $orSql?' AND ':'').($andSql?"($andSql)":'');
		//else
		//	$sql = ' 1=1 ';

		return $sql;
	}
	
	/**
	 * Builds a LIKE based WHERE expression for each of the columns
	 * @see		sqlLikeExpr()
	 * @param	array	String indexed array, columns as keys and searchstrings as values
	 * @param	string	Logical operator to use between columns
	 * @param	string	Logical operator to use between search terms
	 * @param	int		Minimum length of search term
	 * @return	string	Expression that can be used in a sql query
	 */
	static function sqlLikeColExpr($cols, $colCmp = 'OR', $cmp = 'AND', $minLen = 3) {
		foreach ($cols as $colName => $colSearchString) {
			$sqlExpr[] = SyndDBLib::sqlLikeExpr($colSearchString, array($colName), $cmp, $minLen);
		}
		return " (".implode(") $colCmp \n(", $sqlExpr).") ";
	}

	/**
	 * Generates a where statement from an associative array
	 * @static
	 * @access	public
	 * @param	array	Associative array of values
	 * @return	string
	 */
	static function sqlWhere($vals, $quote = true) {
		$sql = null;
		foreach ($vals as $key => $value) {
			if (null != $sql) 
				$sql .= ' AND ';
			if (true == $quote)
				$value = null!==$value?SyndDBLib::quote($value):'NULL';
			$sql .= $key.' = '.(null!==$value?$value:'NULL');
		}
		return $sql;
	}

	/**
	 * Generates an update statement from an associative array
	 * @static
	 * @access	public
	 * @param	array	Associative array of values
	 * @return	string
	 */
	static function sqlUpdate($vals, $quote = true) {
		$sql = null;
		foreach ($vals as $key => $value) {
			if (null != $sql) 
				$sql .= ', ';
			if (true == $quote)
				$value = null!==$value?SyndDBLib::quote($value):'NULL';
			$sql .= $key.' = '.(null!==$value?$value:'NULL');
		}
		return $sql;
	}
	
	/**
	 * Generates column for an INSERT statement
	 * @static
	 * @access	public
	 * @param	string	Associative array of data
	 * @return	string
	 */
	static function sqlCols($data) {
		return implode(", ", array_keys($data));
	}

	/**
	 * Generates values for an INSERT statement
	 * @static
	 * @access	public
	 * @param	string	Associative array of data
	 * @return	string	Statement in the form
	 *					"'Johansson, Mikael', 'di99ohmi@chl.chalmers.se'"
	 */
	static function sqlValues($data, $quote = true) {
		$sql = null;
		foreach ($data as $value) {
			if (null != $sql) 
				$sql .= ', ';
			if (true == $quote && null!==$value)
				$value = SyndDBLib::quote($value);
			$sql .= null!==$value?$value:'NULL';
		}
		return $sql;
	}
	
	static function sqlIn($vals) {
		return implode(',', SyndLib::quote($vals));
	}
}

