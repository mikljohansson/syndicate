<?php

/**
 * HTTP protocol utility class
 *
 * @access		public
 * @package		synd.core.lib
 * @see			HttpConnection
 */
class SyndHTTP {
	/**
	 * Checks if the client is a mobile device (cellphone, ...)
	 * @return	bool
	 */
	static function isMobileDevice() {
		if (isset($_SERVER['HTTP_ACCEPT']) && 
			false !== strpos($_SERVER['HTTP_ACCEPT'], 'application/vnd.wap'))
			return true;
		if (isset($_SERVER['HTTP_USER_AGENT']) && (
			false !== strpos($_SERVER['HTTP_USER_AGENT'], 'Profile/MIDP') ||
			false !== strpos($_SERVER['HTTP_USER_AGENT'], 'Opera Mini') ||
			false !== strpos($_SERVER['HTTP_USER_AGENT'], 'J2ME')))
			return true;
		return false;
	}

	/**
	 * Checks if the client is a handheld (pda, ...)
	 * @return	bool
	 */
	static function isHandheldDevice() {
		if (isset($_SERVER['HTTP_USER_AGENT']) && (
			false !== strpos($_SERVER['HTTP_USER_AGENT'], 'Windows CE') ||
			false !== strpos($_SERVER['HTTP_USER_AGENT'], 'Symbian OS')))
			return true;
		return false;
	}

	/**
	 * Estimates client's actual connection speed.
	 * @param	array	Sorted array of possible connection speeds
	 * @return	integer
	 */
	static function getSpeed($levels = array(56,128,512)) {
		$bandwidth = SyndHTTP::getBandwidth();
		foreach ($levels as $level) {
			if ($bandwidth <= $level)
				return $level;
		}
		return end($levels);
	}
	
	/**
	 * Measured client bandwidth in kilobits per second.
	 * @return	integer
	 */
	static function getBandwidth($default = 56) {
		static $hook = false;
		if (isset($_SESSION['synd']['public']['bandwidth']))
			return round($_SESSION['synd']['public']['bandwidth'] / 1024);
		return $default;
	}
}
