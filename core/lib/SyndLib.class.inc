<?php
/**
 * SyndLib general library functions
 * @package		synd.core.lib
 * @filesource
 */
require_once 'core/i18n.inc';

/**
 * General utility class
 * @access		public
 * @package		synd.core.lib
 */
class SyndLib {
	private static $_hooks = array();
	private static $_dirty = array();
	
	/**
	 * Load average (5min), normalized to a 0-5 exponential scale
	 * @return	integer
	 */
	static function loadLevel() {
		static $load = -1;
		if (-1 == $load) {
			$levels = explode(' ', file_get_contents('/proc/loadavg'));
			$load = round($levels[1]*$levels[1]*5);
		}
		return $load;
	}
	
	/**
	 * Runs a hook
	 *
	 * Runs all the methods attached to the specified hook and returns
	 * the result, defaults to null
	 *
	 * @param	string	Hook name
	 * @param	mixed	Hook parameters
	 * @return	mixed
	 */
	static function runHook($name) {
		if (!isset(self::$_hooks[$name]))
			return null;
		
		$args = func_get_args();
		$args[0] =& $result;
		$hooks = self::$_hooks[$name];

		do {
			self::$_dirty[$name] = array();
			foreach ($hooks as $callback) {
				if (call_user_func_array($callback, $args))
					break 2;
			}
		} while (!empty(self::$_dirty[$name]) && $hooks = self::$_dirty[$name]);

		unset(self::$_dirty[$name]);
		return $result;
	}

	/**
	 * Runs a hook
	 *
	 * Runs all the methods attached to the specified hook and returns
	 * the result, defaults to null
	 *
	 * @param	string	Hook name
	 * @param	mixed	Default result
	 * @param	mixed	Hook parameters
	 * @return	mixed
	 */
	static function runPreloadedHook($name, $result) {
		if (!isset(self::$_hooks[$name]))
			return $result;
		
		$args = func_get_args();
		array_shift($args);
		$args[0] =& $result;
		$hooks = self::$_hooks[$name];

		do {
			self::$_dirty[$name] = array();
			foreach ($hooks as $callback) {
				if (call_user_func_array($callback, $args))
					break 2;
			}
		} while (!empty(self::$_dirty[$name]) && $hooks = self::$_dirty[$name]);

		unset(self::$_dirty[$name]);
		return $result;
	}

	/**
	 * Attach a method to a hook
	 * @see		call_user_func()
	 * @param	string	Hook name
	 * @param	mixed	Method callback (see call_user_func() for reference)
	 * @param	integer	Priority, higher prio gets run first
	 */
	static function attachHook($name, $callback, $priority = 0) {
		assert('is_callable($callback)');
		if (!isset(self::$_hooks[$name]))
			self::$_hooks[$name] = array();

		while (isset(self::$_hooks[$name][$priority]))
			$priority--;

		self::$_hooks[$name][$priority] = $callback;
		krsort(self::$_hooks[$name]);
		
		if (isset(self::$_dirty[$name])) {
			self::$_dirty[$name][$priority] = $callback;
			krsort(self::$_dirty[$name]);
		}
	}
	
	static function getVar($key) {
		return self::runHook('variable_get', 'global', $key);
	}

	static function setVar($key, $value) {
		return self::runHook('variable_set', 'global', $key, $value);
	}
	
	/**
	 * Uses the 'instance' hook to load an instance
	 * @param	string		Id of the requested instance
	 * @return	object
	 */
	static function getInstance($id) {
		return self::runHook('instance', $id);
	}
	
	/**
	 * Uses the 'instances' hook to load a list of instances
	 * @param	array	Ids of the requested instance
	 * @return	array	List of instances indexed by their ids
	 */
	static function getInstances($ids) {
		return empty($ids) ? array() : (array)self::runHook('instances', $ids);
	}
	
	/**
	 * @access	private
	 */
	static function _callback_instance(&$result, $id) {
		if (0 === strpos($id, 'mplex;')) {
			foreach (explode(';', substr($id, strpos($id,';')+1)) as $sid) {
				if (null != $result && method_exists($result, '_hook_loadInstance'))
					$result = $result->_hook_loadInstance($sid);
				else 
					$result = self::runHook('instance', $sid);
				if (null == $result)
					break;
			}
			return null != $result;
		}
	}

	/**
	 * @access	private
	 */
	static function _callback_instances(&$result, $ids) {
		foreach ($ids as $id) {
			if (!isset($result[$id]) && null !== ($instance = self::runHook('instance', $id)))
				$result[$id] = $instance;
		}
		return true;
	}
	
	/**
	 * Checks if a value is an error
	 * @param	mixed	Value to check
	 */
	static function isError($error) {
		return ($error instanceof Exception);
	}
	
	/**
	 * Loads an extension
	 * @see		dl()
	 * @param	string	Name of extension excluding fileending {.so, .dll}
	 * @return	bool	Return true on success, false on failure
	 */
	static function dl($extension) {
		$file = (PHP_SHLIB_SUFFIX=='dll'?'php_':'').$extension.'.'.PHP_SHLIB_SUFFIX;
		return file_exists(ini_get('extension_dir').'/'.$file) && @dl($file);
	}

	/**
	 * Generic hash function
	 * @param	mixed, ...
	 * @return	string
	 */
	static function hash() {
		return md5(serialize(func_get_args()));
	}
	
	/**
	 * Hook for implementing locale modules
	 *
	 * Use this method from .inc files to avoid dependancy on 
	 * TemplateLib.inc, use the regular Template::translate() from .tpl files.
	 *
	 * @param	string	String to translate and parameters to sprintf
	 * @return	string
	 */
	static function translateArgs($args) {
		if (count($args) < 2)
			return null != ($result = self::runHook('text', $args[0])) ? $result : $args[0];
		if (null != ($result = self::runHook('text', $args[0])))
			$args[0] = $result;
		return call_user_func_array('sprintf', $args);
	}

	/**
	 * Hook for implementing locale modules
	 *
	 * Use this method from .inc files to avoid dependancy on 
	 * TemplateLib.inc, use the regular Template::translate() from .tpl files.
	 *
	 * @param	string	String to translate, process, ...
	 * @return	string
	 */
	static function translate() {
		$args = func_get_args();
		return SyndLib::translateArgs($args);
	}

	/**
	 * Hook for implementing locale modules. Also HTML escapes the result
	 *
	 * Use this method from .inc files to avoid dependancy on 
	 * TemplateLib.inc, use the regular Template::text() from .tpl files.
	 *
	 * @param	string	String to translate and parameters to sprintf
	 * @return	string
	 */
	static function textArgs($args) {
		if (count($args) < 2)
			return null != ($result = self::runHook('text', $args[0])) ? $result : $args[0];
		if (null != ($result = self::runHook('text', $args[0])))
			$args[0] = $result;
		for ($i = 1, $last = count($args); $i < $last; $i++)
			$args[$i] = synd_htmlspecialchars($args[$i]);
		return call_user_func_array('sprintf', $args);
	}
	
	/**
	 * Hook for implementing locale modules. Also HTML escapes the result
	 *
	 * Use this method from .inc files to avoid dependancy on 
	 * TemplateLib.inc, use the regular Template::text() from .tpl files.
	 *
	 * @param	string	String to translate, process, ...
	 * @return	string
	 */
	static function text() {
		$args = func_get_args();
		return SyndLib::textArgs($args);
	}

	/**
	 * Recursive trim()
	 * @see		trim()
	 * @param	mixed	Value to trim
	 * @param	string	Characters to trim
	 * @return	mixed
	 */
	static function trim($str, $chars = " \r\n\t") {
		if (is_array($str)) {
			foreach (array_keys($str) as $key)
				$str[$key] = self::trim($str[$key], $chars);
		}
		else if (is_string($str))
			return trim($str, $chars);
		return $str;
	}
	
	/**
	 * is_string() && !is_numeric() && !empty()
	 * @param	string
	 * @return	bool
	 */
	static function isString($str) {
		return is_string($str) && !is_numeric($str) && !empty($str);
	}

	/**
	 * Searches the PATH environment variable and $synd_config['path'] for an executable
	 * @param	string	Name of executable
	 * @return	string	Returns the absolute path or null if not found
	 */
	static function which($name) {
		global $synd_config;
		if (!empty($synd_config['path'])) {
			foreach (explode(':', $synd_config['path']) as $directory) {
				if (file_exists($file = rtrim($directory,'/').'/'.$name))
					return $file;
			}
		}
		if (false != ($path = getenv('PATH'))) {
			foreach (explode(':', $path) as $directory) {
				if (file_exists($file = rtrim($directory,'/').'/'.$name))
					return $file;
			}
		}
		return null;
	}

	static function fitInterval($offset, $count, $interval = 20) {
		if ($count <= $interval)
			return array(0, $count);
		$diff = floor($interval/2);
		if ($offset >= $diff && $offset <= $count-$diff)
			return array($offset-$diff, $offset+$diff);
		if ($offset < $diff)
			return array(0, $interval);
		return array($count-$interval, $count);
	}
	

	/**
	 * Returns the extension part of a filename
	 * @param	string	File
	 * @return	string
	 */
	static function fileExtension($file) {
		$info = pathinfo($file);
		return isset($info['extension']) ? 
			$info['extension'] : null;
	}

	/**
	 * Chops the extension from a file
	 * @param	string	Filename/Path
	 * @return	string
	 */
	static function chopExtension($file) {
		$len = strlen(self::fileExtension($file));
		return $len ? substr($file, 0, strlen($file)-$len-1) : $file;
	}
	
	static function file_total_size($file) {
		if (file_exists($file)) {
			if (is_dir($file)) {
				$size = 0;
				$list = self::scanDirectory($file);
				foreach ($list as $sub)
					$size += self::file_total_size($sub);
				return $size;
			}
			else if (is_file($file))
				return filesize($file);
		}
		return 0;
	}

	/**
	 * Deletes a file or directory
	 * @param	string	Path to file/dir
	 * @param	string	Recursivly delete subdirectories
	 */
	static function unlink($file, $recurse = false) {
		if (is_link($file) || is_file($file))
			return unlink($file);
		if (is_dir($file)) {
			if ($recurse) {
				if (false === ($dir = opendir($file = rtrim($file, '/\\'))))
					return false;
				while(false !== ($sub = readdir($dir))) {
					if ('.' != $sub && '..' != $sub && !self::unlink($file.DIRECTORY_SEPARATOR.$sub, $recurse))
						return false;
				}
				if (!closedir($dir))
					return false;
			}
			return rmdir($file);
		}
		return !file_exists($file);
	}

	/**
	 * Generates the absolute path to a temporary file
	 * @param	string	Filename prefix
	 */
	static function mktemp($prefix = '') {
		global $synd_config;
		return $synd_config['dirs']['cache'].$prefix.md5(uniqid(''));
	}

	/**
	 * Returns a list of absolute paths to files and directories
	 * @param	string	Path to directory
	 * @return	array
	 */
	static function scanDirectory($dir) {
		$list = array();
		$dir = rtrim($dir, '/\\');

		if (file_exists($dir) && false !== ($dh = @opendir($dir))) {
			while (false !== ($file = readdir($dh))) {
				if ('.' == $file || '..' == $file) 
					continue;
				$list[] = $dir.DIRECTORY_SEPARATOR.$file;
			}
		}

		return $list;
	}

	/**
	 * Writes a string to a file.
	 *
	 * Mode determines if file is overwritten or appended to. Returns
	 * true on success, false on failure.
	 *
	 * @see		fopen()
	 * @param	string	Path to file
	 * @param	string	String to write
	 * @param	string	Mode to open file in {'w','a'}
	 * @return	bool
	 */
	static function file_put_contents($file, $content, $mode = 'w') {
		if (false !== ($fp = fopen($file, $mode)) && false !== fwrite($fp, $content) && fclose($fp))
			return true;
		return false;
	}

	/**
	 * Create a directory structure. 
	 *
	 * Tries to recursivly create a directory structure. Returns true
	 * on success, false on failure.
	 *
	 * @param	string	Directory to create
	 * @return	bool
	 */
	static function createDirectory($dir) {
        if (file_exists($dir) && is_dir($dir))
        	return true;

		$parts = preg_split('/\\'.DIRECTORY_SEPARATOR.'+/', $dir, -1, PREG_SPLIT_NO_EMPTY);
		$create = DIRECTORY_SEPARATOR == $dir{0} ? DIRECTORY_SEPARATOR : '';

		// Do not attempt to make directories outside of open_basedir
		$openBasedir = ini_get('open_basedir');
		if(!empty($openBasedir))
			$openBasedirs = explode('WIN' == strtoupper(substr(PHP_OS,0,3)) ? ';' : ':', $openBasedir);

		foreach ($parts as $part) {
			if (!file_exists($create .= $part)) {
				if (!empty($openBasedirs)) {
					foreach ($openBasedirs as $openBasedir) {
						if (substr($create.DIRECTORY_SEPARATOR, 0, strlen($openBasedir)) == $openBasedir)
							break;
						break 2;
					}
				}

				if (!mkdir($create, 0775))
					break;
			}
			$create .= DIRECTORY_SEPARATOR;
		}
		
        return file_exists($dir) && is_dir($dir);
	}

	/**
	 * Invokes a method on each object in a list.
	 *
	 * Returns a list with the return values of each call indexed
	 * with the same key as its node. Does not make references!
	 *
	 * @see		MethodIterator
	 * @param	array	List of objects
	 * @param	string	Method to invoke
	 * @param	mixed	$a,...	Arguments to the member-function
	 * @return	array
	 */
	static function invoke($objects, $method) {
		$args = array_slice(func_get_args(), 2);
		$results = array();
		foreach ($objects as $key => $object)
			$results[$key] = call_user_func_array(array($object, $method), $args);
		return $results;
	}
	
	/**
	 * Sorts a list of object on the result of a method call.
	 * @param	array	List of objects
	 * @param	string	Method to invoke
	 * @param	mixed	$a,...	Arguments to the member-function
	 */
	static function sort($objects, $method = 'toString') {
		$args = array_slice(func_get_args(), 2);
		uasort($objects, array(new SyndLibComparisonDelegate($method, $args), 'compare'));
		return $objects;
	}

	/**
	 * Filters a list of objects on the result of a method call.
	 *
	 * Applies the function to each of the nodes with the supplied
	 * arguments and returns a list with those that returned true.
	 *
	 * Example:
	 *  $filteredList = SyndLib::filter($list, 'isPermitted', 'read');
	 *
	 * @param	array	List of objects
	 * @param	string	Method to invoke
	 * @param	mixed	$a,...	Arguments to the member-function
	 * @return	array
	 */
	static function filter($objects, $method) {
		$result = array();
		$args = array_slice(func_get_args(), 2);
		
		foreach ($objects as $key => $object) {
			if (call_user_func_array(array($object, $method), $args))
				$result[$key] = $object;
		}
		
		return $result;
	}

	/**
	 * Collects a datamember from each object in a list
	 *
	 * Returns a list with the member indexed with the same key as
	 * its node. Does not make references!
	 *
	 * @param	array	List of objects
	 * @param	string	Member to collect
	 * @return	array
	 */
	static function collect($objects, $member) {
		$result = array();
		foreach ($objects as $key => $object)
			$result[$key] = $object->$member;
		return $result;
	}
	
	/**
	 * Returns the key of the minimum value
	 * @see		MethodObjectIterator
	 * @param	array	Values or iterator to traverse
	 * @return	mixed
	 */
	static function min(Iterator $values) {
		$key = null;
		$opt = null;
		
		for ($values->rewind(); $values->valid(); $values->next()) {
			if ($values->current() < $opt || null === $key) {
				$key = $values->key();
				$opt = $values->current();
			}
		}
		
		return $key;
	}

	/**
	 * Returns the key of the maximum value
	 * @see		MethodObjectIterator
	 * @param	array	Values or iterator to traverse
	 * @return	mixed
	 */
	static function max(Iterator $values) {
		$key = null;
		$opt = null;
		
		for ($values->rewind(); $values->valid(); $values->next()) {
			if ($values->current() > $opt || null === $key) {
				$key = $values->key();
				$opt = $values->current();
			}
		}
		
		return $key;
	}

	/**
	 * Sums a number of values
	 * @see		MethodIterator
	 * @param	array	Values or iterator to traverse
	 * @return	mixed
	 */
	static function sum($values) {
		$sum = null;
		foreach ($values as $value)
			$sum += $value;
		return $sum;
	}

	/**
	 * Returns the supported interfaces in lowercase
	 * @param	object	The object to the fetch interfaces for
	 * @return	array
	 */
	static function getInterfaces($instance) {
		$interfaces = array();
		for ($class = get_class($instance); null != $class; $class = get_parent_class($class))
			$interfaces[] = strtolower($class);
		return $interfaces;
	}
	
	/**
	 * Creates a nested array by using the first args as keys and the last as value
	 * map('foo', 'bar', 'foobar') == array('foo' => array('bar' => 'foobar'))
	 * @param	string	$key,...	Keys to be used in nested arrays
	 * @param	mixed	$value		Value to assign to key in innermost array
	 * @return	array
	 */
	static function map() {
		$args = func_get_args();
		$result = array_pop($args);
		while (!empty($args))
			$result = array(array_pop($args) => $result);
		return $result;
	}
	
	/**
	 * @see		reset()
	 */
	static function reset($a) {
		return is_array($a) ? reset($a) : null;
	}
	
	/**
	 * Reference preserving end()
	 * @see		end()
	 * @param	array
	 * @return	mixed
	 */
	static function end($a) {
		return end($a);
	}
	
	/**
	 * Reference preserving array_pop()
	 * @see		array_pop()
	 * @param	array
	 * @return	mixed
	 */
	static function &array_pop(&$list) {
		$end = self::end($list);
		array_pop($list);
		return $end;
	}
	
	static function array_set_current(&$list, $key){
		if (null != $key) {
			reset($list);
			while(false !== current($list) && key($list) != $key)
				next($list);
		}
	}

	static function &array_prev($list, $key) {
		self::array_set_current($list, $key);
		if (null != $key)
			prev($list);
		if (key($list))
			return $list[key($list)];
		$result = null;
		return $result;
	}
	
	static function &array_next($list, $key) {
		self::array_set_current($list, $key);
		if (null != $key)
			next($list);
		if (key($list))
			return $list[key($list)];
		$result = null;
		return $result;
	}

	static function array_filter($list, $function) {
		static $lambda = array();
		if (!is_array($list))
			return $list;
		$key = self::hash($function);
		if (!isset($lambda[$key])) {
			if (is_array($function))
				$lambda[$key] = create_function('$a', 'return '.implode('($a) && ',$function).'($a);');
			else
				$lambda[$key] = create_function('$a',"return $function(\$a);");
		}
		return array_filter($list, $lambda[$key]);
	}

	/**
	 * Alternative array_merge_recursive that overwrites existing keys instead of
	 * creating a subarray and appending an additional value.
	 * @param	array	$a,...	Arrays to merge
	 * @return	array
	 */
	static function array_merge_recursive() {
		$args = func_get_args();
		$soft = is_bool(end($args)) ? array_pop($args) : true;
		$result = array_shift($args);

		while (null != ($arg = array_shift($args))) {
			foreach (array_keys($arg) as $key) {
				if (isset($result[$key]) && isset($arg[$key]) && is_array($result[$key]) && is_array($arg[$key]))
					$result[$key] = self::array_merge_recursive($result[$key], $arg[$key], $soft);
				else if (is_int($key) && isset($result[$key]) && $soft)
					$result[] = $arg[$key];
				else
					$result[$key] = $arg[$key];
			}
		}
		
		return $result;
	}

	/**
	 * Merges the specified arrays by reference
	 * @param	array	$a,...	Arrays to merge
	 * @return	array
	 */
	static function array_merge_assoc() {
		$args = func_get_args();
		$result = array();
		for ($i=0; $i<count($args); $i++) {
			if (is_array($args[$i])) {
				foreach (array_keys($args[$i]) as $key) {
					if (is_numeric($key))
						$result[] = $args[$i][$key];
					else
						$result[$key] = $args[$i][$key];
				}
			}
		}
		return $result;
	}

	/**
	 * Returns the key=>value pairs from the first array whose keys exist
	 * in all arrays.
	 * @see		array_intersect()
	 * @param	array	$a,...	Arrays to intersect
	 * @return	array
	 */
	static function array_kintersect() {
		$args = func_get_args();
		$index = count($args)-1;
		$strip = is_bool($args[$index]) ? $args[$index--] : false;

		$klist = array_keys($args[$index]);
		while ($index-- > 0)
			$klist = array_intersect($klist, array_keys($args[$index]));

		$vals = array();
		foreach ($klist as $key) {
			if (false == $strip || null != $args[0][$key])
				$vals[$key] = $args[0][$key];
		}

		return $vals;
	}

	/**
	 * Returns the key=>value pairs from the first array whose keys does
	 * not exist in any of the other arrays
	 * @see		array_diff()
	 * @param	array	$a,...	Arrays to diff
	 * @return	array
	 */
	static function array_kdiff() {
		$args = func_get_args();
		$vals  = array_shift($args);
		$index = count($args)-1;

		foreach (array_keys($vals) as $key) {
			for ($i = $index; $i >= 0; $i--) {
				if (isset($args[$i][$key])) {
					unset($vals[$key]);
					break;
				}
			}
		}

		return $vals;
	}

	/**
	 * Prepends a value (string) to all the values in an array, recurses subarrays
	 * @param	array	Array to process
	 * @param	mixed	Value to prepend
	 * @param	mixed	Callback to determine if a value should be prefixed
	 * @return	array
	 */
	static function array_prepend($vals, $prefix, $decider = 'is_string') {
		foreach (array_keys($vals) as $key) {
			if (call_user_func($decider, $vals[$key]))
				$vals[$key] = $prefix.$vals[$key];
			else if (is_array($vals[$key]))
				$vals[$key] = self::array_prepend($vals[$key], $prefix);
		}
		return $vals;
	}

	/**
	 * Returns the keys in an array, returns an empty array if no array was supplied
	 * @param	array
	 * @return	array
	 */
	static function array_keys($vals) {
		return is_array($vals) ? array_keys($vals) : array();
	}

	/**
	 * Implodes an array, returns null if no array was supplied
	 * @see		implode()
	 * @param	array	Pieces
	 * @return	string
	 */
	static function array_implode($glue, $pieces) {
		return is_array($pieces) ? implode($glue, $pieces) : null;
	}

	/**
	 * Collects a value from a list of subarrays
	 * @param	array	Array of arrays
	 * @param	mixed	Index of value to collect
	 * @return	array
	 */
	static function array_collect($list, $index) {
		$result = array();
		foreach (array_keys($list) as $key)
			$result[$key] = $list[$key][$index];
		return $result;
	}

	/**
	 * Intersperses an array with the specified value at every
	 * second index.
	 * array_intersperse(array(1,2,3), 'foo') == array(1,'foo',2,'foo',3)
	 * @param	array	Array to intersperse
	 * @param	mixed	Value to place at every second index
	 * @return	array
	 */
	static function array_intersperse($arr, $str) {
		$result = array();
		foreach ($arr as $index => $value) {
			$result[] = $value;
			if (isset($arr[$index+1]))
				$result[] = $str;
		}
		return $result;
	}
	
	/**
	 * Multiple column sorting
	 * 
	 * For some reason the array to be sorted can not be passed by
	 * reference or a segfault occur in uasort
	 * 
	 * @param	array	The array to be sorted
	 * @param	array	The ordering
	 * @param	mixed	Compare callback
	 * @return	array
	 */
	static function array_multisort($list, $order, $callback = null) {
		global $_syndlib_callback, $_syndlib_order;
		
		if (null == $callback)
			$_syndlib_callback = array('SyndLib','_callback_compare');
		else
			$_syndlib_callback = $callback;
		$_syndlib_order = $order;
		
		uasort($list, array('SyndLib','_callback_multisort'));
		return $list;
	}
	
	/**
	 * @access	private
	 */
	static function _callback_compare($a, $b, $key) {
		return strnatcasecmp(
			isset($a[$key]) ? $a[$key] : null, 
			isset($b[$key]) ? $b[$key] : null);
	}
	
	/**
	 * @access	private
	 */
	static function _callback_multisort($a, $b) {
		global $_syndlib_callback, $_syndlib_order;
		for ($i=0, $cnt=count($_syndlib_order); $i<$cnt; $i++) {
			if (is_string($_syndlib_order[$i])) {
				if (0 != ($value = call_user_func($_syndlib_callback, $a, $b, $_syndlib_order[$i]))) {
					if (isset($_syndlib_order[$i+1]) && SORT_DESC == $_syndlib_order[$i+1])
						return -$value;
					return $value;
				}
			}
		}
		return 0;
	}
	
	/**
	 * Transforms the PHP $_FILES format into something more intuitive
	 * @param	array	The $_FILES array
	 * @return	array
	 */
	static function filesTransform($files) {
		$result = array();

		foreach ($files as $key => $value) {
			if (is_array($value[key($value)])) {
				foreach ($value as $attribute => $branch) {
					$result[$key] = isset($result[$key]) ? 
						array_merge_recursive($result[$key], self::_filesTransform($branch, $attribute)) :
						self::_filesTransform($branch, $attribute);
				}
			}
			else {
				$result[$key] = $value;
			}
		}
		
		return $result;
	}

	private static function _filesTransform($files, $attribute) {
		$result = array();
		
		foreach ($files as $key => $value) {
			if (is_array($value)) {
				$result[$key] = isset($result[$key]) ? 
					array_merge_recursive($result[$key], self::_filesTransform($value, $attribute)) :
					self::_filesTransform($value, $attribute);
			}
			else {
				$result[$key][$attribute] = $value;
			}
		}
		
		return $result;
	}

	/**
	 * Executes an external program and return the output
	 * @param	string	Command to execute using proc_open()
	 * @param	string	Buffer to write on STDIN
	 * @param	bool	Trigger failure on binary output
	 * @param	integer	Max output to read (null for unlimited)
	 * @return	string
	 */
	static function execute($cmd, $input = null, $skipBinary = false, $maxLength = null) {
		$spec = array(
			0 => array('pipe','r'),
			1 => array('pipe','w'),
			2 => array('pipe','w'));

		$pd = proc_open($cmd, $spec, $pipes);
		if (!is_resource($pd)) {
			trigger_error("Could not open process '$cmd'", E_USER_WARNING);
			return null;
		}
		
		fclose($pipes[2]);
		$stdout = array($pipes[1]);
		$output = null;
		
		if (null != $input)
			$stdin = array($pipes[0]);
		else {
			fclose($pipes[0]);
			$stdin = null;
		}

		$length = strlen($input);
		$oob = null;

		for ($i=0, $timeout = time()+10; !feof($pipes[1]);) {
			$read = $stdout;
			$write = $stdin;
			
			stream_select($read, $write, $oob, 1);
			
			// Write a chunk of data
			if (!empty($write) && $i < $length) {
				if (false === ($i += fwrite($pipes[0], substr($input, $i, 8192)))) {
					trigger_error('Could not write to STDIN.', E_USER_WARNING);
					$output = null;
					break;
				}
				else if ($i >= $length) {
					fclose($pipes[0]);
					$stdin = null;
				}
			}

			// Read any output
			if (!empty($read)) {
				while (!feof($pipes[1]) && null != ($output .= fread($pipes[1], 8192))) {
					if ($skipBinary && false !== strpos($output, "\0")) {
						trigger_error('Binary file detected, skipping.', E_USER_WARNING);
						@fclose($pipes[0]);
						$output = null;
						break 2;
					}
					if (null !== $maxLength && strlen($output) >= $maxLength) {
						@fclose($pipes[0]);
						break 2;
					}
				}
			}

			// Break after 10 seconds without IO
			if (!empty($read) || !empty($write))
				$timeout = time()+10;
			else if ($timeout < time()) {
				@fclose($pipes[0]);
				break;
			}
		}

		fclose($pipes[1]);
		proc_close($pd);

		return $output;
	}
}

if (!function_exists('debug')) {
	/**
	 * Debug dump/output of varibles, triggers warning
	 * @see		print_r()
	 * @uses	debug_fixup()
	 * @param	mixed	$v,...	Varibles to be printed
	 */
	function debug() {
		global $synd_config;
		if (!isset($_SERVER['REMOTE_ADDR'], $synd_config['debug_allowed_ips'][$_SERVER['REMOTE_ADDR']]))
			return;

		$args = func_get_args();
		$args = debug_fixup($args);
		trigger_error(null, E_USER_WARNING);
		print '<pre>';

		if (is_array($args)) {
			foreach ($args as $arg) {
				print_r($arg);
				print ' ';
			}
		}

		print '</pre><br />';
		flush();
	}

	/**
	 * Fixes the values of varibles so they look nicer on output
	 * @access	private
	 * @param	mixed	Varibles to fix, walks arrays recursivly
	 * @return	mixed
	 */
	function debug_fixup($args, $rec = 0) {
		if ($rec > 10)
			return null;

		if (is_array($args)) {
			foreach (array_keys($args) as $key)
				$args[$key] = debug_fixup($args[$key], $rec+1);
		}
		else if (is_bool($args))
			return $args ? 'true':'false';
		else if (null === $args)
			return 'null';
		else if (is_string($args))
			return synd_htmlspecialchars($args);
		
		return $args;
	}
}

/**
 * Iterates over the results of method calls
 */
class MethodIterator extends IteratorIterator {
	protected $_method = null;
	protected $_args = null;
	
	/**
	 * @param	Iterator	Object iterator to wrap
	 * @param	string		Method to call on objects from inner iterator
	 * @param	array		Optional parameters to method
	 */
	function __construct(Iterator $it, $method, Array $args = array()) {
		parent::__construct($it);
		$this->_method = $method;
		$this->_args = $args;
	}
	
	/**
	 * Returns the result from the method call on the current object
	 * @return	mixed
	 */
	function current() {
		return call_user_func_array(array(parent::current(), $this->_method), $this->_args);
	}
}

/**
 * Iterates over the results of method calls using the object as key
 */
class MethodObjectIterator extends IteratorIterator {
	protected $_method = null;
	protected $_args = null;
	
	/**
	 * @param	Iterator	Object iterator to wrap
	 * @param	string		Method to call on objects from inner iterator
	 * @param	array		Optional parameters to method
	 */
	function __construct(Iterator $it, $method, Array $args = array()) {
		parent::__construct($it);
		$this->_method = $method;
		$this->_args = $args;
	}
	
	/**
	 * Returns the result from the method call on the current object
	 * @return	mixed
	 */
	function current() {
		return call_user_func_array(array(parent::current(), $this->_method), $this->_args);
	}

	/**
	 * Returns the current object
	 * @return	object
	 */
	function key() {
		return parent::current();
	}
}

/**
 * Iterates over a member variable
 */
class MemberIterator extends IteratorIterator {
	protected $_member = null;
	
	/**
	 * @param	Iterator	Object iterator to wrap
	 * @param	string		Member to return from objects from inner iterator
	 */
	function __construct(Iterator $it, $member) {
		parent::__construct($it);
		$this->_member = $member;
	}
	
	/**
	 * Returns the specified member from the current object
	 * @return	mixed
	 */
	function current() {
		return parent::current()->{$this->_member};
	}
}

/**
 * Filters object based on the result from a method call
 */
class MethodFilterIterator extends FilterIterator {
	function __construct(Iterator $it, $method, $args = null) {
		parent::__construct($it);
		$this->_iterator = $it;
		$this->_method = $method;
		$this->_args = (array)$args;
	}

	function accept() {
		return (bool)call_user_func_array(array($this->current(), $this->_method), $this->_args);
	}
}

/**
 * Domain object match decider
 */
interface MatchDecider {
	/**
	 * Should the domain object be accepted
	 * @param	DomainObject	The domain object to check
	 * @return	bool
	 */
	function accept($object);
}

/**
 * Calls a method on the object to decide inclusion
 */
class MethodDecider implements MatchDecider {
	protected $_method = null;
	protected $_args = null;
	
	function __construct($method, $args = null) {
		$this->_method = $method;
		$this->_args = (array)$args;
	}
	
	function accept($object) {
		return call_user_func_array(array($object, $this->_method), $this->_args);
	}
}

/**
 * Always returns true
 */
class BooleanDecider implements MatchDecider {
	function accept($object) {
		return true;
	}
}

/**
 * Null object iterator aggregate
 */
class EmptyIteratorAggregate implements IteratorAggregate {
	function getIterator() {
		return new EmptyIterator();
	}
}

/**
 * @access	private
 */
class SyndLibComparisonDelegate {
	private $_method = null;
	private $_args = null;
	
	function __construct($method, $args) {
		$this->_method = $method;
		$this->_args = $args;
	}
	
	function compare($a, $b) {
		return strcasecmp(
			call_user_func_array(array($a, $this->_method), $this->_args), 
			call_user_func_array(array($b, $this->_method), $this->_args));
	}
}

class Delegate {
	private $_callback = null;
	private $_args = null;
	
	function __construct($callback, $args = array()) {
		$this->_callback = $callback;
		$this->_args = $args;
	}
	
	function run() {
		$args = func_get_args();
		return call_user_func_array($this->_callback, array_merge($this->_args, $args));
	}
	
	/**
	 * Creates a new delegate
	 * @param	callback	The callback to run
	 * @param	array		Arguments to prepend to argument list when running callback
	 */
	static function factory($callback, $args = array()) {
		return array(new self($callback, $args), 'run');
	}
}

/**
 * Starts the profiler. 
 *
 * Output will be displayed using the register_shutdown_function 
 * hook. Requires the PEAR::Benchmark package
 */
function profiler_start() {
	if (!isset($GLOBALS['_profiler_instance'])) {
		global $_profiler_instance;
		require_once 'Benchmark/Profiler.php';
		$_profiler_instance = new Benchmark_Profiler(true);
	}
}

/**
 * Enter a profiler section
 * @param	string	Name of section (function, method, ...)
 */
function profiler_enter($section) {
	if (isset($GLOBALS['_profiler_instance']))
		$GLOBALS['_profiler_instance']->enterSection($section);
}

/**
 * Leave a profiler section
 * @param	string	Name of section (function, method, ...)
 */
function profiler_leave($section) {
	if (isset($GLOBALS['_profiler_instance']))
		$GLOBALS['_profiler_instance']->leaveSection($section);
}

function basename_safe($path) {
        $path = rtrim($path,'/');
        $path = explode('/',$path);
        return end($path);
}

SyndLib::attachHook('instance', array('SyndLib', '_callback_instance'), -100);
SyndLib::attachHook('instances', array('SyndLib', '_callback_instances'), -100);
