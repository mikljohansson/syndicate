<?php
require_once 'core/lib/SyndLib.class.inc';

/**
 * @access		public
 * @package		synd.core.lib
 */
interface ClassLoader {
	/**
	 * Resolves a classname into a class file and includes it
	 * @throws	RuntimeException
	 * @param	string	Name of class
	 * @return	string	Returns path to classfile, false if none found
	 */
	function load($class);
	
	/**
	 * Returns a namespace for this loader
	 * @return	string
	 */
	function getNamespace();
}

/**
 * Recurses a directory tree looking for matching class files
 */
class DirectoryClassLoader implements ClassLoader {
	protected $_patterns = null;
	protected $_paths = null;
	protected $_match = null;
	protected $_mangle = null;
	
	/**
	 * @see		preg_match()
	 * @param	array	Paths to recurse into, e.g. explode(':', get_include_path())
	 * @param	array	Regexp patterns that the absolute filepath must match
	 * @param	string	Pattern to match the classname, e.g. "/^synd_[a-z]+_/S"
	 * @param	string	Pattern to mangle the classname, e.g. "/^synd_[a-z]+_/S"
	 */
	public function __construct(Array $paths, Array $patterns, $match = null, $mangle = null) {
		$this->_paths = $paths;
		$this->_patterns = $patterns;
		$this->_match = $match;
		$this->_mangle = $mangle;
	}

	public function load($class) {
		profiler_enter(__METHOD__);
		
		if (null == $this->_match || preg_match($this->_match, $class)) {
			$patterns = array();
			$filename = null != $this->_mangle ? preg_replace($this->_mangle, '', $class) : $class;
			
			foreach ($this->_patterns as $pattern)
				$patterns[] = str_replace('{$CLASS}', $filename, $pattern);
			
			foreach ($this->_paths as $path) {
				if (0 !== strpos($path, '.') && false !== ($file = $this->_load($class, $patterns, new RecursiveDirectoryIterator($path)))) {
					profiler_leave(__METHOD__);
					return $file;
				}
			}

			trigger_error("Failed to load class '$class'", E_USER_NOTICE);
		}
		
		profiler_leave(__METHOD__);
		return false;
	}
	
	public function getNamespace() {
		return md5(implode(':', $this->_paths));
	}
	
	protected function _load($class, $patterns, $iterator) {
		for ($iterator->rewind(); $iterator->valid(); $iterator->next()) {
			if (0 !== strpos($iterator->getFilename(), '.')) {
				if ($iterator->isDir()) {
					if (false !== ($file = $this->_load($class, $patterns, $iterator->getChildren())))
						return $file;
				}
				else {
					$file = realpath($iterator->getPathname());
					foreach ($patterns as $pattern) {
						if (preg_match($pattern, $file) && (include_once($file)) && class_exists($class, false))
							return $file;
					}
				}
			}
		}
		return false;
	}
}

/**
 * Caches class paths to disk
 */
class CachingClassLoader implements ClassLoader {
	protected $_next = null;
	protected $_strategy = null;
	protected $_cache = null;
	protected $_dirty = false;
	
	/**
	 * @see		CacheStrategyManager::localized()
	 * @param	ClassLoader		Next loader in chain
	 * @param	CacheStrategy	Localized cache strategy to use
	 */
	public function __construct(ClassLoader $next, CacheStrategy $strategy) {
		$this->_next = $next;
		$this->_strategy = $strategy;
	}
	
	public function __destruct() {
		if ($this->_dirty)
			$this->_strategy->set('clspath.'.$this->_next->getNamespace(), $this->_cache);
	}

	public function load($class) {
		profiler_enter(__METHOD__);
		
		if (null === $this->_cache && false == ($this->_cache = $this->_strategy->get('clspath.'.$this->_next->getNamespace())))
			$this->_cache = array();
			
		if (isset($this->_cache[$class])) {
			if (file_exists($this->_cache[$class]) && (include_once($this->_cache[$class])) && class_exists($class, false)) {
				profiler_leave(__METHOD__);
				return $this->_cache[$class];
			}
			
			unset($this->_cache[$class]);
			$this->_dirty = true;
		}
		
		if (false !== ($file = $this->_next->load($class))) {
			$this->_cache[$class] = $file;
			$this->_dirty = true;
		}
		
		profiler_leave(__METHOD__);
		return $file;
	}
	
	public function getNamespace() {
		return $this->_next->getNamespace();
	}
}
