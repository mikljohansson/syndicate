<?php

require_once 'core/lib/SyndLib.class.inc';

class synd_type_file extends SyndType {
	/**
	 * Relative path to file
	 * @access	private
	 */
	var $_file = null;

	/**
	 * Created timestamp
	 * @access	private
	 */
	var $_created = null;
	
	/**
	 * Size of file in bytes
	 * @access	private
	 */
	var $_size = null;

	function __construct($oid = null) {
		parent::__construct($oid);
		$this->_created = time();
	}

	static function __factory($class, $args = null) {
		global $synd_config;
		if (!isset($args[0]) || !file_exists($args[0]) || !is_file($args[0]))
			return null;

		if (!isset($args[1]) || null == ($extension = SyndLib::fileExtension($args[1])))
			$extension = SyndLib::fileExtension($args[0]);
		switch (strtolower($extension)) {
			case 'jpg':
			case 'jpeg':
			case 'png':
			case 'gif':
				$file = parent::__factory(SyndType::loadClass('image'));
				break;

			case 'mpg':
			case 'mpeg':
			case 'avi':
				$file = parent::__factory(SyndType::loadClass('movie'));
				break;

			case 'mp3':
				$file = parent::__factory(SyndType::loadClass('mp3'));
				break;

			default:
				$file = parent::__factory($class);
		}
		
		// Create relative path and filename
		if (!isset($args[1]) || null == ($name = basename_safe($args[1])))
			$name = basename_safe($args[0]);
		$file->_file = $file->_relative().$file->_rewrite($name);
		$file->_size = filesize($args[0]);
		
		// Transfer file to LOB storage
		if (!SyndLib::runHook('lob_storage_set', $file->_file, $args[0])) {
			trigger_error('Failed to store file', E_USER_WARNING);
			return null;
		}
		
		return $file;
	}

	function __clone() {
		global $synd_config;
		$file = $this->_file;
		parent::__clone();
		
		$this->_file = $this->_relative().basename_safe($file);
		$this->_created = time();

		SyndLib::runHook('lob_storage_copy', $file, $this->_file);
	}

	function toString() {
		return basename_safe($this->_file);
	}

	/**
	 * @access	private
	 */
	function _rewrite($name) {
		return preg_match('/^\.ht|\.(php\d?|phtml|phtm|shtml|pl|py|cgi)$/', $name) ? $name.'.txt' : $name;
	}
	
	/**
	 * Deletes this file from storage
	 * @return	boolen	Returns true on success, false on failure
	 */
	function delete() {
		if (SyndLib::runHook('lob_storage_delete', $this->_file)) {
			$this->_file = null;
			return true;
		}
		return false;
	}
	
	function uri() {
		return SyndLib::runHook('lob_storage_uri', $this->_file);
	}

	function path() {
		return SyndLib::runHook('lob_storage_uri', $this->_file);
	}
	
	function getContents() {
		return file_get_contents($this->uri());
	}
	
	function getSize() {
		// TODO: remove this deprecated inode fetching
		if ((null === $this->_size || !is_numeric($this->_size)) && null != ($info = SyndLib::runHook('lob_storage_stat',$this->_file)))
			$this->_size = $info['size'];
		return $this->_size;
	}
	
	function getCreated() {
		return $this->_created;
	}

	static function nullInstance() {
		return new _synd_type_file_null();
	}
	
	/**
	 * @access	private
	 */
	function _relative() {
		return $this->classId().'/'.$this->objectId().'/';
	}
}
/**
 * @access	private
 */
class _synd_type_file_null extends synd_type_file {
	function isNull() {
		return true;
	}
}
