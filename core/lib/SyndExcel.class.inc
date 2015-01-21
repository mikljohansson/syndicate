<?php

/** 
 * Excel spreadsheet generation library
 *
 * @access		public
 * @package		synd.core.lib
 */
class SyndExcel {
	static function quote($value) {
		return preg_replace('/[\r\n]+/', ' ', str_replace('"','\"', $value));
	}
	
	static function cell($value = null) {
		return '"'.self::quote($value)."\"\t";
	}
}
