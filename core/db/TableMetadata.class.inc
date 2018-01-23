<?php

/**
 * Database table metadata
 * @access		public
 * @package		synd.core.db
 */
class TableMetadata {
	/**
	 * @access	private
	 */
	var $_name = null;
	
	/**
	 * @access	private
	 */
	var $_columns = array();
	
	/**
	 * @access	private
	 */
	var $_relations = array();

	/**
	 * @param	string	Table name
	 */
	function __construct($name) {
		$this->_name = $name;
	}

	/**
	 * Add a column to this table
	 * @param	DatabaseColumn
	 */
	function addColumn($column) {
		$this->_columns[$column->toString()] = $column;
	}
	
	/**
	 * Returns columns in this table
	 * @return	array	Array of DatabaseColumn
	 */
	function getColumns() {
		return $this->_columns;
	}
	
	/**
	 * Returns names of columns in this table
	 * @return	array
	 */
	function getColumnNames() {
		return array_keys($this->_columns);
	}

	/**
	 * Returns column default values
	 * @return	array
	 */
	function getDefaultValues() {
		$result = array();
		foreach ($this->_columns as $name => $column) 
			$result[$name] = $column->getDefaultValue();
		return $result;
	}
	
	/**
	 * Add a relation to this table
	 * @param	DatabaseConstraint	Source contraint
	 * @param	DatabaseConstraint	Target contraint
	 */
	function addRelation($source, $target) {
		$this->_relations[] = array($source, $target);
	}

	/**
	 * Returns foreign keys from this table
	 * @return	array
	 */
	function getRelations() {
		return $this->_relations;
	}
	
	/**
	 * Validates data against this schema
	 * @param	array
	 * @return	array
	 */
	function validate($data) {
		$errors = array();
		
		foreach (array_keys($this->_columns) as $key) {
			$name = $this->_columns[$key]->getName();
			if (isset($data[$name]) && null !== $data[$name] && false !== $data[$name] && '' !== $data[$name]) {
				if (null !== ($error = $this->_columns[$key]->validate($data[$name])))
					$errors[$name] = $error;
			}
			else if ($this->_columns[$key]->isRequired())
				$errors[$name] = SyndLib::translate("Field '%s' is required", SyndLib::translate($name));
		}
		
		return $errors;
	}
}

abstract class DatabaseColumn {
	var $_name = null;
	var $_null = true;
	var $_default = null;
	
	function __construct($name, $null = true, $default = null) {
		$this->_name = $name;
		$this->_null = $null;
		$this->_default = $default;
	}
	
	function toString() {
		return $this->_name;
	}
	
	function getName() {
		return $this->_name;
	}
	
	function getDefaultValue() {
		return $this->_default;
	}
	
	function isRequired() {
		return !$this->_null && null == $this->_default;
	}

	/**
         * Allows binary values to be double encoded if needed (PostgreSQL BYTEA columns)
         */
	function wrap($value) {
		return $value;
	}

	/**
         * Allows binary values to be double encoded if needed (PostgreSQL BYTEA columns)
         */
	function unwrap($value) {
		return $value;
	}
	
	abstract function validate($value);
}

class DatabaseStringColumn extends DatabaseColumn {
	var $_db = null;
	var $_width = null;
	var $_binary = false;
	
	function __construct($db, $name, $null = true, $default = null, $width = 255, $binary = false) {
		parent::__construct($name, $null, $default);
		$this->_db = $db;
		$this->_width = (int)$width;
		$this->_binary = $binary;
	}
	
	function validate($value) {
		if (is_object($value) || is_array($value)) 
			return null;
		if ($this->_width && strlen($value) > $this->_width)
			return SyndLib::translate("Value in field '%s' exceeds the maximum length of %s characters", $this->toString(), $this->_width);
		if (!$this->_binary && false !== strpos($value, "\0"))
			return SyndLib::translate("Field '%s' may not contain binary data", $this->toString());
		return null;
	}
	
	function getWidth() {
		return $this->_width;
	}

        function wrap($value) {
                return $this->_binary ? $this->_db->wrapBinary($value) : parent::unwrap($value);
        }

        function unwrap($value) {
                return $this->_binary ? $this->_db->unwrapBinary($value) : parent::unwrap($value);
        }
}

class DatabaseLobColumn extends DatabaseStringColumn {}

class DatabaseIntegerColumn extends DatabaseColumn {
	var $_bits = null;
	var $_unsigned = false;

	/**
	 * @param	string	Column name
	 * @param	bool	NULL allows
	 * @param	integer	Default value
	 * @param	integer	Bits used for storage
	 */
	function __construct($name, $null = true, $default = null, $bits = 32, $unsigned = false) {
		parent::__construct($name, $null, null == $default ? $default : (int)$default);
		$this->_bits = $bits;
		$this->_unsigned = $unsigned;
	}
	
	function validate($value) {
		if (!is_numeric($value))
			return SyndLib::translate("Non numeric value ('%s') in field '%s' given", tpl_chop($value,10), $this->toString());
		
		$negative = '-' == substr($value, 0, 1);
		if ($this->_unsigned && $negative)
			return SyndLib::translate("Value in field '%s' must be positive", $this->toString());
		
		if ($this->_bits <= 32) {
			$min = $this->_unsigned ? 0 : -pow(2, $this->_bits) / 2;
			$max = $this->_unsigned ? pow(2, $this->_bits) - 1 : pow(2, $this->_bits) / 2 - 1;
			if ($value < $min || $value > $max)
				return SyndLib::translate("Value in field '%s' out of bounds (%d..%d)", $this->toString(), $min, $max);
		}
	}
}

class DatabaseNumberColumn extends DatabaseColumn {
	var $_precision = null;
	var $_scale = null;

	/**
	 * @param	string	Column name
	 * @param	bool	NULL allows
	 * @param	integer	Default value
	 * @param	integer	Bits used for storage
	 */
	function __construct($name, $null = true, $default = null, $precision = 11, $scale = 0) {
		parent::__construct($name, $null, null == $default ? $default : (int)$default);
		$this->_precision = $precision;
		$this->_scale = $scale;
	}
	
	function validate($value) {
		if (!is_numeric($value))
			return SyndLib::translate("Non numeric value ('%s') in field '%s' given", tpl_chop($value,10), $this->toString());

		if (null !== $this->_precision && strlen(false !== ($i = strpos($value,'.')) ? substr($value,0,$i) : $value) > $this->_precision)
			return SyndLib::translate("Value '%s' out of bounds (max %d digits)", $this->toString(), $this->_precision);
		if (null !== $this->_scale && false !== ($i = strpos($value,'.')) && strlen($value)-$i > $this->_scale)
			return SyndLib::translate("Value '%s' out of bounds (max %d decimals)", $this->toString(), $this->_scale);
	}
}

class DatabaseFloatColumn extends DatabaseColumn {
	var $_unsigned = false;

	function __construct($name, $null = true, $default = null, $unsigned = false) {
		parent::__construct($name, $null, null == $default ? $default : (float)$default);
		$this->_unsigned = $unsigned;
	}

	function validate($value) {
		if (!is_numeric($value))
			return SyndLib::translate("Non numeric value ('%s') in field '%s' given", tpl_chop($value,10), $this->toString());
		
		$negative = '-' == substr($value, 0, 1);
		if ($this->_unsigned && $negative)
			return SyndLib::translate("Value in field '%s' must be positive", $this->toString());
	}
}

class DatabaseDatetimeColumn extends DatabaseColumn {
	var $_format = null;
	
	function __construct($name, $null = true, $default = null, $format = 'YYYY-MM-DD HH:MM:SS') {
		parent::__construct($name, $null, $default);
		$this->_format = $format;
	}
	
	function validate($value) {
		$format = '/'.preg_replace('/(yy|mm|dd|hh|ss)/i', '\\d{2}', $this->_format).'/';
		if (!preg_match($format, $value, $matches)) 
			return SyndLib::translate("Invalid date format (%s) in field '%s'", $this->_format, $this->toString());
	}
}

class DatabaseConstraint {
	/**
	 * @access	private
	 */
	var $_name = null;

	/**
	 * @access	private
	 */
	var $_table = null;

	/**
	 * @access	private
	 */
	var $_columns = null;

	/**
	 * @param	string	Name of constraint
	 * @param	string	Name of table
	 * @param	array	Constraint columns
	 */
	function __construct($name, $table, $columns) {
		$this->_name = $name;
		$this->_table = $table;
		$this->_columns = $columns;
	}
	
	function getName() {
		return $this->_name;
	}

	function getTable() {
		return $this->_table;
	}

	function getColumns() {
		return $this->_columns;
	}
}
