<?php
require_once 'design/gui/TableWidget.class.inc';

/**
 * Spreadsheet builder interface
 *
 * All coordinates are zero-based and given as integers counted from
 * the top left corner of the spreadsheet.
 *
 * @access		public
 * @package		synd.design
 */
class ISpreadsheetBuilder {
	/**
	 * Renders contents to string
	 */
	function toString() {}

	/**
	 * Begins a new row
	 */
	function tr() {}

	/**
	 * Output a table header cell
	 * @param	string	Cell value
	 * @param	string	CSS class to use
	 * @param	string	Uri to link to
	 * @param	string	Column description
	 */
	function th($value, $class = null, $uri = null, $title = null) {}

	/**
	 * Output a normal cell
	 * @param	string	Cell value
	 * @param	string	CSS class to use
	 * @param	string	Uri to link cell to
	 */
	function td($value, $class = null, $uri = null) {}

	/**
	 * Output a summary over a number of blocks
	 *
	 * The list of blocks to summarize is encoded as
	 * <code>
	 * array(
	 *  array(x1, y1, x2, y2),
	 *  array(x1, y1, x2, y2),
	 * )
	 * </code>
	 *
	 * @param	array	List of blocks to summarize
	 * @param	string	CSS class to use
	 * @param	string	Uri to link cell to
	 */
	function sum($blocks, $class = null, $uri = null) {}
	
	/**
	 * Output an average over a number of blocks
	 *
	 * The list of blocks to summarize is encoded as
	 * <code>
	 * array(
	 *  array(x1, y1, x2, y2),
	 *  array(x1, y1, x2, y2),
	 * )
	 * </code>
	 *
	 * @param	array	List of blocks to summarize
	 * @param	string	CSS class to use
	 * @param	string	Uri to link cell to
	 * @param	integer	Number of decimals to round to
	 */
	function avg($blocks, $class = null, $uri = null, $decimals = 0) {}

	/**
	 * Returns the current row index
	 * @return	integer
	 */
	function getCurrentCol() {}

	/**
	 * Returns the current row index
	 * @return	integer
	 */
	function getCurrentRow() {}
}

class ExcelSpreadsheetBuilder extends ISpreadsheetBuilder {
	/**
	 * @access	private
	 */
	var $_col = 0;
	
	/**
	 * @access	private
	 */
	var $_row = 0;

	function toString() {}
	
	function tr() {
		$this->_col = 0;
		$this->_row++;
		print "\r\n";
	}

	function th($value, $class = null, $uri = null, $title = null) {
		$this->td($value);
	}

	function td($value, $class = null, $uri = null) {
		$this->_col++;
		print "\"";
		print $this->quote($value);
		print "\"\t";
	}

	function sum($blocks, $class = null, $uri = null) {
		$this->_col++;
		$i = 0;
		
		foreach ($blocks as $block) {
			print $i++ ? '+' : '=';
			print 'SUM('.chr(ord('A')+$block[0]).($block[1]+1).':'.chr(ord('A')+$block[2]).($block[3]+1).')';
		}
		
		print "\t";
	}

	function avg($blocks, $class = null, $uri = null, $decimals = 0) {
		$this->_col++;
		$i = 0;
		
		foreach ($blocks as $block) {
			print $i++ ? '+' : '=';
			print "ROUND(AVERAGE(".chr(ord('A')+$block[0]).($block[1]+1).':'.chr(ord('A')+$block[2]).($block[3]+1)."); $decimals)";
		}
		
		print "\t";
	}

	function getCurrentCol() {
		return $this->_col;
	}

	function getCurrentRow() {
		return $this->_row;
	}

	/**
	 * @access	private
	 */
	function quote($value) {
		return preg_replace('/[\r\n]+/', ' ', str_replace('"','\"', $value));
	}
}

class HtmlSpreadsheetBuilder extends ISpreadsheetBuilder {
	/**
	 * @access	private
	 */
	var $_widget = null;
	
	/**
	 * @access	private
	 */
	var $_col = 0;
	
	/**
	 * @access	private
	 */
	var $_row = 0;
	
	/**
	 * @access	private
	 */
	var $_titles = null;
	
	function HtmlSpreadsheetBuilder() {
		$this->_widget = new TableWidget();
		$this->_widget->setRowAttribute($this->_row, 'class', tpl_cycle(array('odd','even')));
		$this->_widget->setAttribute('class', 'Report');
	}
	
	function toString() {
		$result = $this->_widget->toString();
		if (!empty($this->_titles)) {
			$result .= '<ol class="Info">';
			foreach ($this->_titles as $title)
				$result .= "<li>$title</li>";
			$result .= '</ol>';
		}
		return $result;
	}
	
	function tr() {
		$this->_col = 0;
		$this->_row++;
		$this->_widget->setRowAttribute($this->_row, 'class', tpl_cycle(array('odd','even')));
	}
	
	function th($value, $class = null, $uri = null, $title = null) {
		$contents = null != $uri ? "<a href=\"$uri\">$value</a>" : $value;
		
		if (null !== $title) {
			$this->_titles[] = $title;
			$contents .= ' <sup>'.count($this->_titles).'</sup>';
		}
		
		$cell = $this->_widget->add(new TableWidgetCell($this->_row, $this->_col++, $contents, null, 'th'));
		if (null != $class)
			$cell->setAttribute('class', $class);
		$cell->_value = $value;
	}

	function td($value, $class = null, $uri = null) {
		$contents = null != $uri ? "<a href=\"$uri\">$value</a>" : $value;
		$cell = $this->_widget->add(new TableWidgetCell($this->_row, $this->_col++, $contents));
		if (null != $class)
			$cell->setAttribute('class', $class);
		$cell->_value = $value;
	}
	
	function sum($blocks, $class = null, $uri = null) {
		$this->_widget->add(new HtmlSpreadsheetSum($this->_widget, $this->_row, $this->_col++, $blocks, $class, $uri));
	}

	function avg($blocks, $class = null, $uri = null, $decimals = 0) {
		$this->_widget->add(new HtmlSpreadsheetAvg($this->_widget, $this->_row, $this->_col++, $blocks, $class, $uri, $decimals));
	}

	function getCurrentCol() {
		return $this->_col;
	}

	function getCurrentRow() {
		return $this->_row;
	}
}

/**
 * @access	private
 */
class HtmlSpreadsheetSum extends TableWidgetCell {
	var $_widget = null;
	var $_blocks = null;
	var $_uri = null;
	
	function HtmlSpreadsheetSum($widget, $row, $col, $blocks, $class = null, $uri = null) {
		parent::TableWidgetCell($row, $col);
		$this->_widget = $widget;
		$this->_blocks = $blocks;
		$this->_uri = $uri;
		if (null != $class)
			$this->setAttribute('class', $class);
	}
	
	function toString() {
		$value = null;
		
		foreach ($this->_blocks as $block) {
			for ($col=$block[0]; $col<=$block[2]; $col++) {
				for ($row=$block[1]; $row<=$block[3]; $row++) {
					$cell = $this->_widget->get($row, $col);
					$value += $cell->_value;
				}
			}
		}
		
		return null != $this->_uri ? "<a href=\"{$this->_uri}\">".round($value,1)."</a>" : round($value,1);
	}
	
	function getTagname() {
		return 'th';
	}
}

/**
 * @access	private
 */
class HtmlSpreadsheetAvg extends TableWidgetCell {
	var $_widget = null;
	var $_blocks = null;
	var $_uri = null;
	var $_decimals = 0;
	
	function HtmlSpreadsheetAvg($widget, $row, $col, $blocks, $class = null, $uri = null, $decimals) {
		parent::TableWidgetCell($row, $col);
		$this->_widget = $widget;
		$this->_blocks = $blocks;
		$this->_uri = $uri;
		if (null != $class)
			$this->setAttribute('class', $class);
		$this->_decimals = $decimals;
	}
	
	function toString() {
		$value = null;
		$count = 0;
		
		foreach ($this->_blocks as $block) {
			for ($col=$block[0]; $col<=$block[2]; $col++) {
				for ($row=$block[1]; $row<=$block[3]; $row++) {
					$cell = $this->_widget->get($row, $col);
					if (is_numeric($cell->_value)) {
						$value += $cell->_value;
						$count++;
					}
				}
			}
		}
		
		return null != $this->_uri ? 
			"<a href=\"{$this->_uri}\">".($count ? round($value/$count,$this->_decimals) : 0)."</a>" : 
			$count ? round($value/$count,$this->_decimals) : 0;
	}
	
	function getTagname() {
		return 'th';
	}
}
