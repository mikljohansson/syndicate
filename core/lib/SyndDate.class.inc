<?php

/**
 * Date/Time utility class
 *
 * @static
 * @access		public
 * @package		synd.core.lib
 */
class SyndDate {
	/**
	 * Checks if a date is valid
	 * @param	string	Date (YYYY-MM-DD)
	 * @return	bool
	 */
	static function checkdate($date) {
		if (!is_numeric($date)) {
			$expl = explode('-', $date);

			if (3 != count($expl)) return false;
			if (!strlen($expl[0]) || !is_numeric($expl[0])) return false;
			if (!strlen($expl[1]) || !is_numeric($expl[1])) return false;
			if (!strlen($expl[2]) || !is_numeric($expl[2])) return false;
		
			return checkdate($expl[1], $expl[2], $expl[0]);
		}
		return -1 != @strtotime($date);
	}

	static function checktime($time) {
		return preg_match('/(\d{2}):?(\d{2})/', $time, $matches) &&
			($matches[1] >= 0 && $matches[1] < 24 && $matches[2] >= 0 && $matches[2] <= 59 || 24 == $matches[1] && 0 == $matches[2]);
	}

	/**
	 * Returns unix timestamp indicating the start of the month
	 * @param	integer	Unix timestamp
	 * @return	integer
	 */
	static function startOfYear($ts) {
		return strtotime(date('Y-01-01 00:00:00', $ts));
	}

	/**
	 * @see		SyndDate::startOfYear()
	 */
	static function endOfYear($ts) {
		return strtotime(date('Y-12-31 23:59:59', $ts));
	}

	/**
	 * @see		SyndDate::startOfYear()
	 */
	static function startOfMonth($ts) {
		return strtotime(date('Y-m-01 00:00:00', $ts));
	}

	/**
	 * @see		SyndDate::startOfYear()
	 */
	static function endOfMonth($ts) {
		return strtotime(date('Y-m-t 23:59:59', $ts));
	}

	/**
	 * @see		SyndDate::startOfYear()
	 */
	static function startOfWeek($ts) {
		if (date('w',$ts)==1)
			return strtotime('00:00:00', $ts);
		return strtotime('last Monday 00:00:00', $ts);
	}
	
	/**
	 * @see		SyndDate::startOfYear()
	 */
	static function endOfWeek($ts) {
		return SyndDate::startOfWeek($ts) + 3600*24*7 - 1;
	}

	/**
	 * @see		SyndDate::startOfYear()
	 */
	static function startOfDay($ts) {
		return strtotime('00:00:00', $ts);
	}
	
	/**
	 * @see		SyndDate::startOfYear()
	 */
	static function endOfDay($ts) {
		return strtotime('23:59:59', $ts);
	}

	/**
	 * @see		SyndDate::startOfYear()
	 */
	static function startOfHour($ts) {
		return strtotime(date('Y-m-d H:00',$ts), $ts);
	}

	/**
	 * @see		SyndDate::startOfYear()
	 */
	static function endOfHour($ts) {
		return SyndDate::startOfHour($ts)+3599;
	}

	/**
	 * Returns the number of days between two timestamps
	 * @param	integer	Timestamp 1
	 * @param	integer	Timestamp 2
	 * @return	integer
	 */
	static function lengthDays($ts1, $ts2) {
		$ts1 = $ts1 - $ts1%(3600*24);
		$ts2 = $ts2 - $ts2%(3600*24);
		return ceil(abs($ts1-$ts2)/(3600*24));
	}
	
	/**
	 * Returns the number of hours between two timestamps
	 * @param	integer	Timestamp 1
	 * @param	integer	Timestamp 2
	 * @return	integer
	 */
	static function lengthHours($ts1, $ts2) {
		$ts1 = $ts1 - $ts1%3600;
		$ts2 = $ts2 - $ts2%3600;
		return ceil(abs($ts1-$ts2)/3600);
	}
	
	/**
	 * Returns Today|Tomorrow|YYYY-MM-DD
	 * @param	integer	Timestamp
	 * @param	string	Today text
	 * @param	string	Tomorrow text
	 * @return	string
	 */
	static function switchDays($ts, $today = 'Today', $tomorrow = 'Tomorrow') {
		if (date('Ymd',$ts) == date('Ymd') && null != $today)
			return $today;
		if (date('Ymd',$ts) == date('Ymd',time()+3600*24) && null != $tomorrow)
			return $tomorrow;
		return date('Y-m-d', $ts);
	}

	static function evalDuration($duration) {
		if (preg_match('/^(\d+[*+\-])+\d+$/', $duration))
			return eval("return $duration;");
		return strlen($duration) ? $duration : null;
	}
	
	static function durationExpr($duration) {
		$result = $duration = SyndDate::evalDuration($duration);
		
		if (floor($duration/60) >= 2) {
			$result = '60*'.floor($duration/60);
			if ($duration%60)
				$result .= '+'.($duration%60);
		}
		
		return $result;
	}
}

