<?php
require_once 'core/model/Collection.class.inc';

class synd_type_null_collection extends SyndType implements Collection {
	function isNull() {return true;}

	function toString() {return '';}
	function getCount() {return 0;}
	function getContents($offset = 0, $limit = null) {return array();}
	function getFilteredCount($classes) {return 0;}
	function getFilteredContents($classes) {return array();}
	
	function getCalendarCapabilities() {return array();}
	function appendChild($child) {}
}
