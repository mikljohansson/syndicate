<?php

/**
 * Table layoyt widget
 *
 * Coordinates are zero based from top-left corner of table.
 *
 * @access		public
 * @package		synd.design
 */
class TableWidget {
	/**
	 * @access	private
	 */
	var $_table = array();
	
	/**
	 * @access	private
	 */
	var $_attributes = null;
	
	/**
	 * Row attributes
	 * @access	private
	 */
	var $_rowAttributes = null;
	
	/**
	 * Default cell attributes
	 * @access	private
	 */
	var $_cellAttributes = null;
	
	/**
	 * Default cell attributes callback
	 * @access	private
	 */
	var $_cellAttributesCallback = null;
	
	function TableWidget($attributes = null) {
		$this->_attributes = $attributes;
	}
	
	/**
	 * Renders this table to HTML
	 * @return	string
	 */
	function toString() {
		if (empty($this->_table))
			return '';
		
		// Calculate table extent
		$rows = 0;
		foreach (array_keys($this->_table) as $col)
			$rows = max($rows, $this->_table[$col]->getRowCount());
		$cols = max(array_keys($this->_table))+1;
		
		// Render all cells
		$html = "<table".TableWidget::implode($this->_attributes).">\r\n";
		for ($row=0; $row<$rows; $row++) {
			$html .= "\t<tr".TableWidget::implode(isset($this->_rowAttributes[$row])?$this->_rowAttributes[$row]:null).">\r\n";
			for ($col=0; $col<$cols; $col++)
				$html .= isset($this->_table[$col]) ? 
					$this->_table[$col]->toString($row, $col) : 
					"\t\t<td".TableWidget::implode($this->_getDefaultAttributes($row,$col)).">&nbsp;</td>\r\n";
			$html .= "\t</tr>\r\n";
		}

		$html .= '</table>';
		return $html;
	}
	
	/**
	 * Set an attribute on the <table> element
	 * @param	string	Attribute name
	 * @param	string	Value of attribute
	 */
	function setAttribute($attribute, $value) {
		$this->_attributes[$attribute] = $value;
	}
	
	/**
	 * Set an attribute on a <tr> element
	 * @param	integer	Row index
	 * @param	string	Attribute name
	 * @param	string	Value of attribute
	 */
	function setRowAttribute($row, $attribute, $value) {
		$this->_rowAttributes[$row][$attribute] = $value;
	}

	/**
	 * Set a default attribute used for empty cells
	 * @param	string	Attribute name
	 * @param	string	Value of attribute
	 */
	function setDefaultAttribute($attribute, $value) {
		$this->_cellAttributes[$attribute] = $value;
	}
	
	/**
	 * Returns the default cell attributes
	 * @return	array
	 */
	function getDefaultAttributes() {
		return (array)$this->_cellAttributes;
	}
	
	/**
	 * Returns the default cell attributes
	 * @access	protected
	 * @param	integer		Row being rendered
	 * @param	integer		Column being rendered
	 * @return	array
	 */
	function _getDefaultAttributes($row, $col) {
		return null !== $this->_cellAttributesCallback ? 
			call_user_func_array($this->_cellAttributesCallback, array($this, $row, $col)) : 
			(array)$this->_cellAttributes;
	}
	
	/**
	 * Set a default cell attributes callback
	 *
	 * The callback must follow the prototype:
	 *  array myCallback($widget, $row, $col)
	 *
	 * @param	array	Callback to set
	 */
	function setDefaultAttributesCallback($callback) {
		$this->_cellAttributesCallback = $callback;
	}
	
	/**
	 * Add a table cell
	 * @param	ITableWidgetCell	Cell to add
	 * @return	ITableWidgetCell	Returns the added cell
	 */
	function add($cell) {
		$row = $cell->getRow();
		$col = $cell->getCol();
		
		// Add cell to all columns it spans
		for ($i=0,$colspan=$cell->getColspan(); $i<$colspan; $i++) {
			if (!isset($this->_table[$col+$i]))
				$this->_table[$col+$i] = new TableWidgetColumn($this);
			$this->_table[$col+$i]->set($row, $cell);
		}
		
		return $cell;
	}

	/**
	 * Returns an individual cell
	 * @param	integer	Row index
	 * @param	integer	Column index
	 * @return	ITableWidgetCell
	 */
	function get($row, $col) {
		if (!isset($this->_table[$col]) || null === ($result = $this->_table[$col]->get($row)))
			$result = new TableWidgetCell($row, $col, null);
		return $result;
	}

	/**
	 * Implodes html attributes to key="value"
	 * @param	array	Attributes as key=>value pairs
	 * @return	string
	 */
	static function implode($attributes) {
		if (empty($attributes))
			return '';
		$result = ' ';
		foreach ($attributes as $key => $value)
			$result .= $key.'="'.$value.'" ';
		return rtrim($result);
	}
}

/**
 * Table cell interface
 * @access		public
 * @package		synd.design
 */
class ITableWidgetCell {
	/**
	 * Returns cell content
	 * @return	string
	 */
	function toString() {}
	
	/**
	 * Returns cell tagname {td, th}
	 * @return	string
	 */
	function getTagname() {}

	/**
	 * Sets a cell attribute
	 * @param	string	Attribute name
	 * @param	string	Value of attribute
	 */
	function setAttribute($attribute, $value) {}

	/**
	 * Returns cell attributes as key=>value pairs
	 * @return	array
	 */
	function getAttributes() {}
	
	/**
	 * Returns cell start row
	 * @return	integer
	 */
	function getRow() {}
	
	/**
	 * Returns cell start column
	 * @return	integer
	 */
	function getCol() {}

	/**
	 * Returns cell rowspan
	 * @return	integer
	 */
	function getRowspan() {}

	/**
	 * Returns cell colspan
	 * @return	integer
	 */
	function getColspan() {}
}

/**
 * Generic table cell implementation
 */
class TableWidgetCell extends ITableWidgetCell {
	/**
	 * @access	private
	 */
	var $_row = null;

	/**
	 * @access	private
	 */
	var $_col = null;

	/**
	 * @access	private
	 */
	var $_contents = null;

	/**
	 * @access	private
	 */
	var $_attributes = null;
	
	/**
	 * @access	private
	 */
	var $_tagname = null;

	/**
	 * @param	integer	Row index
	 * @param	integer	Column index
	 * @param	string	Cell contents
	 * @param	array	Attributes as key=>value pairs
	 * @param	string	Cell tagname {td, th}
	 */
	function TableWidgetCell($row, $col, $contents = '&nbsp;', $attributes = null, $tagname = 'td') {
		$this->_row = $row;
		$this->_col = $col;
		$this->_contents = $contents;
		$this->_attributes = $attributes;
		$this->_tagname = $tagname;
	}
	
	function toString() {
		return $this->_contents;
	}
	
	function getTagname() {
		return $this->_tagname;
	}
	
	function getRow() {
		return $this->_row;
	}
	
	function getCol() {
		return $this->_col;
	}

	function setAttribute($attribute, $value) {
		if (false !== ($key = array_search(strtolower($attribute), array_change_key_case((array)$this->_attributes, CASE_LOWER))))
			$attribute = $key;
		$this->_attributes[$attribute] = $value;
	}

	function getAttributes() {
		return (array)$this->_attributes;
	}

	function getAttribute($attribute) {
		$attributes = array_change_key_case((array)$this->_attributes, CASE_LOWER);
		return isset($attributes[strtolower($attribute)]) ? (string)$attributes[strtolower($attribute)] : null;
	}
	
	function getRowspan() {
		return max(1, (integer)$this->getAttribute('rowspan'));
	}

	function getColspan() {
		return max(1, (integer)$this->getAttribute('colspan'));
	}
}

/**
 * @access	private
 */
class TableWidgetColumn {
	var $_parent = null;
	var $_table = array();
	
	function TableWidgetColumn($parent) {
		$this->_parent = $parent;
	}
	
	function toString($row, $col) {
		$html = '';
		for ($i=0,$cols=$this->getImplicitColumnCount(); $i<$cols; $i+=$colspan) {
			if (isset($this->_table[$row][$i])) {
				$attributes = $this->_table[$row][$i]->getAttributes();

				// Remove colspan attribute if conflicts in this column
				if ($cols > 1 && false !== ($key = array_search('colspan',array_change_key_case($attributes, CASE_LOWER))))
					unset($attributes[$key]);
				// Increase colspan if no conflicts for this event
				if ($cols > 1 && 1 == $this->getImplicitColumnCount(
					$this->_table[$row][$i]->getRow(), $this->_table[$row][$i]->getRow() + $this->_table[$row][$i]->getRowspan() - 1))
					$colspan = $attributes['colspan'] = $this->_table[$row][$i]->getColspan() + $cols - 1;
				else {
					$colspan = 1;
				}
					
				// Skip render if implicitly spanned
				if ($row == $this->_table[$row][$i]->getRow() && $col == $this->_table[$row][$i]->getCol()) {
					$html .= "\t\t<".$this->_table[$row][$i]->getTagname().TableWidget::implode($attributes).">";
					$html .= $this->_table[$row][$i]->toString();
					$html .= "</".$this->_table[$row][$i]->getTagname().">\r\n";
				}
			}
			else {
				// Colspan to next cell in this row or end of row
				if (isset($this->_table[$row]) && false !== ($next = TableWidgetColumn::next($this->_table[$row], $i+1)))
					$colspan = $next - $i;
				else
					$colspan = $cols - $i;

				$attributes = (array)$this->_parent->_getDefaultAttributes($row, $col);
				if ($colspan > 1) {
					if (false !== ($key = array_search('colspan',array_change_key_case($attributes, CASE_LOWER))))
						unset($attributes[$key]);
					$attributes['colspan'] = $colspan;
				}
				$html .= "\t\t<td".TableWidget::implode($attributes).">&nbsp;</td>\r\n";
			}
		}
		return $html;
	}
	
	/**
	 * @access	private
	 */
	function next($row, $index) {
		$keys = array_keys($row);
		if (false === ($i = array_search($index, $keys)))
			return false;
		return $keys[$i];
	}
	
	function set($row, $cell) {
		// Find next column index without conflicts
		$col = $this->getImplicitColumnCount($row, $row+$cell->getRowspan()-1);
		// Add cell to all rows on selected column index
		for ($i=0; $i<$cell->getRowspan(); $i++)
			$this->_table[$row+$i][$col] = $cell;
	}
	
	function get($row) {
		if (isset($this->_table[$row]))
			return $this->_table[$row][key($this->_table[$row])];
		$result = null;
		return $result;
	}
	
	/**
	 * Calculates the number of rows to render for this column
	 * @access	private
	 */
	function getRowCount() {
		$rows = 0;
		foreach (array_keys($this->_table) as $row) {
			foreach (array_keys($this->_table[$row]) as $col)
				$rows = max($rows, $this->_table[$row][$col]->getRow() + $this->_table[$row][$col]->getRowspan());
		}
		return $rows;
	}
	
	/**
	 * Returns the implicit column count
	 * @access	private
	 * @param	integer	Start row index
	 * @param	integer	End row index
	 * @return	integer
	 */
	function getImplicitColumnCount($start = 0, $stop = null) {
		if (null === $stop && !empty($this->_table))
			$stop = max(array_keys($this->_table));
		$cols = 0;
		for ($row=$start; $row<=$stop; $row++) {
			if (isset($this->_table[$row]))
				$cols = max($cols, max(array_keys($this->_table[$row]))+1);
		}
		return $cols;
	}
}
