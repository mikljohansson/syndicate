<?php
require_once 'core/lib/SyndDate.class.inc';
require_once 'design/gui/TableWidget.class.inc';

/**
 * Simplifies rendering of calendar views
 * @abstract
 * @access		public
 * @package		synd.design
 */
class CalendarWidget {
	/**
	 * @access	private
	 */
	var $_events = array();
	
	/**
	 * @access	private
	 */
	var $_start = null;

	/**
	 * @access	private
	 */
	var $_stop = null;
	
	/**
	 * @param	integer	Hard beginning timestamp
	 * @param	integer	Hard end timestamp
	 */
	function CalendarWidget($start = null, $stop = null) {
		$this->_start = $start;
		$this->_stop = $stop;
	}
	
	/**
	 * Renders this widget to HTML
	 * @return	string
	 */
	function toString() {
		$table = $this->getTable();
		return $table->toString();
	}
	
	/**
	 * Builds a TableWidget from this calendar view
	 * @return	TableWidget
	 */
	function getTable() {assert(false);}

	/**
	 * Add an event to this widget
	 * @param	CalendarEvent
	 */
	function addEvent(CalendarEvent $event) {
		$this->_events[] = $event;
	}
	
	/**
	 * Returns events
	 * @param	integer	Start timestamp of range to return
	 * @param	integer	End timestamp of range to return
	 */
	function getEvents($start = null, $stop = null) {
		$events = array();
		foreach (array_keys($this->_events) as $key) {
			if (CalendarWidget::intersects($this->_events[$key]->getStart(), $this->_events[$key]->getEnd(), $start, $stop))
				$events[] = $this->_events[$key];
		}
		return $events;
	}
	
	/**
	 * Returns the first timestamp to display
	 * @return	integer
	 */
	function getStart() {
		return null != $this->_start ? $this->_start : min(SyndLib::invoke($this->_events, 'getStart'));
	}
	
	/**
	 * Returns the last timestamp to display
	 * @return	integer
	 */
	function getEnd() {
		return null != $this->_stop ? $this->_stop : max(SyndLib::invoke($this->_events, 'getEnd'));
	}
	
	/**
	 * @access	protected
	 */
	function min($ts1, $ts2) {
		return null !== $ts1 && $ts1 < $ts2 || null === $ts2 ? $ts1 : $ts2;
	}

	/**
	 * @access	protected
	 */
	function max($ts1, $ts2) {
		return null !== $ts1 && $ts1 > $ts2 || null === $ts2 ? $ts1 : $ts2;
	}
	
	/**
	 * @access	protected
	 */
	function intersects($ts1, $ts2, $r1, $r2) {
		return 
			null === $r1 && null === $r2 ||	// Range is infinite
			null === $r1 && $ts2 <= $r2 ||	// Lower bound is infinite 
			null === $r2 && $ts1 >= $r1 ||	// Upper bound is infinite
			$ts1 >= $r1 && $ts1 < $r2 || 	// Start is within bounds
			$ts2 >= $r1 && $ts2 <= $r2 ||	// End is within bounds
			$ts1 <= $r1 && $ts2 >= $r2;		// Bounds are within event
	}

	/**
	 * Adds the column index
	 * @access	protected
	 */
	function _callback_javascript_attributes($widget, $row, $col) {
		return array_merge($widget->getDefaultAttributes(), array(
			'id'			=> ($row-1).'x'.($col-1),
			'row'			=> $row-1,
			'col'			=> $col-1,
			'style'			=> 'cursor:crosshair; -moz-user-select:none;',
			'unselectable'	=> 'on',
			));
	}
}

/**
 * Calendar week view
 */
class CalendarWeekWidget extends CalendarWidget {
	/**
	 * @access	private
	 */
	var $_resolution = null;
	
	/**
	 * @param	integer	Hard beginning timestamp
	 * @param	integer	Hard end timestamp
	 * @param	integer	Resolution in seconds per row {900,1800,3600}
	 */
	function CalendarWeekWidget($start = null, $stop = null, $resolution = 3600) {
		parent::CalendarWidget($start, $stop);
		$this->_resolution = $resolution;
	}

	function getTable() {
		$dayStart = floor($this->getDayStart() / $this->_resolution) * $this->_resolution;
		$dayEnd = ceil($this->getDayEnd() / $this->_resolution) * $this->_resolution;

		$table = new TableWidget(array('class'=>'Week'));
		$table->add(new TableWidgetCell(0, 0, '&nbsp;', null, 'th'));

		// Setup time-of-day column
		for ($i=0, $rowspan=3600/$this->_resolution, $t=$dayStart; $t<$dayEnd; $i+=$rowspan, $t+=3600) {
			$attrib = array('class'=>'Time');
			if ($rowspan > 1) 
				$attrib['rowspan'] = $rowspan;
			$table->add(new TableWidgetCell($i+1, 0, sprintf('%02d:%02d',floor($t/3600),floor($t%3600/60)), $attrib, 'th'));
		}

		// Setup weekday names and render events
		$start = $this->getStart();
		$end = $this->getEnd();

		for ($i=0, $t=strtotime('00:00',$start); $t<$end; $i++, $t=strtotime('+1 days 00:00',$t)) {
			$contents  = "\r\n\t\t\t<div class=\"Weekday\">".ucwords(strftime('%A',$t))."</div>";
			$contents .= "\r\n\t\t\t<div class=\"Date\">".strftime('%Y-%m-%d',$t)."</div>\r\n\t\t";
			$table->add(new TableWidgetCell(0, $i+1, $contents, 
				array('style' => 'width:'.floor(99/ceil(($end-$start)/3600/24)).'%;'), 'th'));

			foreach (array_keys($events = $this->getEvents($t+$dayStart, $t+$dayEnd)) as $key) {
				$evstart = max($t+$dayStart, $events[$key]->getStart());
				$evstop = min($t+$dayEnd, $events[$key]->getEnd());
				$offset = floor(max(0, ($evstart - strtotime('00:00',$evstart) - $dayStart) / $this->_resolution));
				$rowspan = ceil($evstop/$this->_resolution) - floor($evstart/$this->_resolution);

				$attrib = method_exists($events[$key],'getAttributes') ? 
					$events[$key]->getAttributes() : array('class' => 'Event');
				if ($rowspan > 1)
					$attrib['rowspan'] = $rowspan;
				$table->add(new TableWidgetCell($offset+1, $i+1, $events[$key]->toString(), $attrib));
			}
		}

		return $table;
	}
	
	/**
	 * Offset from midnight until start of day
	 * @return	integer
	 */
	function getDayStart() {
		if (null != $this->_start)
			return $this->_start - strtotime('00:00', $this->_start);
		$offset = null;
		foreach (array_keys($this->_events) as $key)
			$offset = CalendarWidget::min($offset, $this->_events[$key]->getStart() - strtotime('00:00', $this->_events[$key]->getStart()));
		return $offset;
	}

	/**
	 * Offset from midnight until start of day
	 * @return	integer
	 */
	function getDayEnd() {
		if (null != $this->_stop)
			return $this->_stop - strtotime('00:00', $this->_stop);
		$offset = null;
		foreach (array_keys($this->_events) as $key)
			$offset = CalendarWidget::max($offset, $this->_events[$key]->getEnd() - strtotime('00:00', $this->_events[$key]->getEnd()));
		return $offset;
	}

	/**
	 * Adds the timestamp attributes
	 * @access	protected
	 */
	function _callback_javascript_attributes($widget, $row, $col) {
		$attributes = parent::_callback_javascript_attributes($widget, $row, $col);
		$dayStart = $this->getDayStart();
		$start = $this->getStart();
		
		$rowStart = strtotime('+'.($col-1).' days 00:00', $start) + $dayStart + ($row - 1) * $this->_resolution;
		$attributes['start'] = $rowStart;
		$attributes['end'] = $rowStart + $this->_resolution;
		return $attributes;
	}
}
