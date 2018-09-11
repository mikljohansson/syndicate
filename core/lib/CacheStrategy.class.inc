<?php

function _synd_cache_jitter($ttl) {
	return $ttl !== null ? rand((int)($ttl * 0.75), $ttl) : null;
}

/**
 * Caching strategy manager
 * @access		public
 * @package		synd.core.lib
 */
abstract class CacheStrategyManager {
	static protected $_strategy = null;
	static protected $_localized = null;

	/**
	 * Autodetects supported caching strategies
	 * @return	CacheStrategy
	 */
	static function factory() {
		global $synd_config;
		if (null == self::$_strategy) {
			if (function_exists('memcache_set') && !empty($synd_config['cache']['memcached']))
				self::$_strategy = new MemcachedStrategy($synd_config['cache']['memcached']);
			else if (function_exists('apc_store'))
				self::$_strategy = new ApcCacheStrategy();
			else if (function_exists('xcache_set'))
				self::$_strategy = new XCacheStrategy();
			else {
				$path = isset($synd_config['dirs']['cache']) ? $synd_config['dirs']['cache'].'cache/' : 
					(isset($_SERVER['SERVER_NAME']) ? "/tmp/{$_SERVER['SERVER_NAME']}/cache/" : '/tmp/local/cache/');
				self::$_strategy = new FilesystemCacheStrategy($path);
			}
		}
		return self::$_strategy;
	}

	/**
	 * Autodetects supported localized (single host) caching strategies
	 * @return	CacheStrategy
	 */
	static function localized() {
		global $synd_config;
		if (null == self::$_localized) {
			if (function_exists('apc_store'))
				self::$_localized = new ApcCacheStrategy();
			else if (function_exists('xcache_set'))
				self::$_localized = new XCacheStrategy();
			else {
				$path = isset($synd_config['dirs']['cache']) ? $synd_config['dirs']['cache'].'cache/' : 
					(isset($_SERVER['SERVER_NAME']) ? "/tmp/{$_SERVER['SERVER_NAME']}/cache/" : '/tmp/local/cache/');
				self::$_localized = new FilesystemCacheStrategy($path);
			}
		}
		return self::$_localized;
	}
}	

/**
 * Caching strategy
 * @access		public
 * @package		synd.core.lib
 */
interface CacheStrategy {
	/**
	 * Retrives a value from cache
	 * @param	mixe	Key or array of keys to retrieve
	 * @return	mixed	Returns false if not found
	 */
	function get($key);

	/**
	 * Stores a value
	 * @param	string	Key to store value under
	 * @param	mixed	Value to store
	 * @param	integer	Time to live in seconds
	 * @return	bool	Returns true on success, false on failure
	 */
	function set($key, $value, $ttl = null);
	
	/**
	 * Stores a value if it doesn't already exist
	 * @param	string	Key to store value under
	 * @param	string	Value to store
	 * @param	integer	Time to live in seconds
	 * @return	bool	Returns true on success, false on failure
	 */
	function add($key, $value, $ttl = null);
	
	/**
	 * Stores a value if it already exists
	 * @param	string	Key to store value under
	 * @param	string	Value to store
	 * @param	integer	Time to live in seconds
	 * @return	bool	Returns true on success, false on failure
	 */
	function replace($key, $value, $ttl = null);

	/**
	 * Increment an integer
	 * @param	string	Key to increment
	 * @param	string	Value to increment
	 * @param	integer	Time to live in seconds
	 * @return	integer	Returns false if the value is not set
	 */
	function increment($key, $value = 1);
	
	/**
	 * Decrement an integer
	 * @param	string	Key to decrement
	 * @param	string	Value to decrement
	 * @param	integer	Time to live in seconds
	 * @return	integer	Returns false if the value is not set
	 */
	function decrement($key, $value = 1);

	/**
	 * Removes a value from cache
	 * @param	string	Key to remove
	 * @return	bool	Returns true on success, false on failure
	 */
	function delete($key);
	
	/**
	 * Clears the entire cache
	 */
	function clear();
}

/**
 * Abstract cache strategy implementation
 * @access		public
 * @package		synd.core.lib
 */
abstract class AbstractCacheStrategy implements CacheStrategy {
	function get($key) {
		if (is_array($key)) {
			$result = false;
			foreach ($key as $k) {
				if (false !== ($value = $this->get($k)))
					$result[$k] = $value;
			}
			return $result;
		}
		return false;
	}
	
	function add($key, $value, $ttl = null) {
		if (false !== $this->get($key))
			return false;
		return $this->set($key, $value, $ttl);
	}
	
	function replace($key, $value, $ttl = null) {
		if (false !== $this->get($key))
			return $this->set($key, $value, $ttl);
		return false;
	}

	function increment($key, $value = 1) {
		if (false !== ($prev = $this->get($key)) && $this->set($key, $prev+$value))
			return $prev+$value;
		return false;
	}

	function decrement($key, $value = 1) {
		if (false !== ($prev = $this->get($key)) && $this->set($key, $prev-$value))
			return $prev-$value;
		return false;
	}
}

class NamespaceCacheStrategy implements CacheStrategy {
	protected $_next = null;
	
	function __construct(CacheStrategy $next) {
		$this->_next = $next;
	}
	
	function get($key) {
		return $this->_next->get($this->_modify($key));
	}

	function set($key, $value, $ttl = null) {
		return $this->_next->set($this->_modify($key), $value, $ttl);
	}

	function add($key, $value, $ttl = null) {
		return $this->_next->add($this->_modify($key), $value, $ttl);
	}
	
	function replace($key, $value, $ttl = null) {
		return $this->_next->replace($this->_modify($key), $value, $ttl);
	}

	function increment($key, $value = 1) {
		return $this->_next->increment($this->_modify($key), $value);
	}

	function decrement($key, $value = 1) {
		return $this->_next->decrement($this->_modify($key), $value);
	}
	
	function delete($key) {
		return $this->_next->delete($this->_modify($key));
	}
	
	function clear($namespace = null) {
		if (null == $namespace)
			$this->_next->clear();
		else
			$this->_next->increment('__ns__'.$namespace);
	}
	
	protected function _modify($key, &$nscnt = null) {
		if (is_array($key)) {
			foreach ($key as $k => $v)
				$key[$k] = $this->_modify($v, $nscnt);
			return $key;
		}
		
		$nsl = null;
		$nsc = null;

		for ($i=0; false !== ($i = strpos($key, '/', $i)); $i++) {
			$ns = '__ns__'.substr($key, 0, $i);
			if (!isset($nscnt[$ns]))
				$nsc[] = $ns;
			$nsl[] = $ns;
		}
		
		if (null != $nsl) {
			if (null != $nsc)
				$nscnt = array_merge((array)$nscnt, $this->_next->get($nsc));

			foreach ($nsl as $ns) {
				if (!isset($nscnt[$ns]))
					$nscnt[$ns] = (int)$this->_next->add($ns, 1);
				$key .= '.'.$nscnt[$ns];
			}
		}
		
		return $key;
	}
}

/**
 * Memcache distributed caching strategy
 *
 * The memcache library provides load balancing, transparent failover
 * and automatic compression.
 *
 * @link	http://www.danga.com/memcached/
 */
class MemcachedStrategy implements CacheStrategy {
	protected $_connection = null;
	protected $_hosts = null;

	function __construct($hosts) {
		$this->_hosts = $hosts;
		$this->_connect();
	}
	
	/**
	 * @access	protected
	 */
	function __sleep() {
		$vars = array_keys(get_object_vars($this));
		unset($vars[array_search('_connection', $vars)]);
		return $vars;
	}
	
	/**
	 * @access	protected
	 */
	function __wakeup() {
		$this->_connect();
	}
	
	function _connect() {
		$this->_connection = new Memcache();
		foreach ($this->_hosts as $host)
			$this->_connection->addServer(substr($host,0,$i=strpos($host,':')), substr($host,$i+1));
		$this->_connection->setCompressThreshold(10000, 0.2);
	}

	function getConnection() {
		return $this->_connection;
	}

	function get($key) {
		return $this->_connection->get($key);
	}

	function set($key, $value, $ttl = null) {
		return $this->_connection->set($key, $value, 0, $ttl);
	}

	function add($key, $value, $ttl = null) {
		return @$this->_connection->add($key, $value, 0, $ttl);
	}
	
	function replace($key, $value, $ttl = null) {
		return @$this->_connection->replace($key, $value, 0, $ttl);
	}

	function increment($key, $value = 1) {
		return $this->_connection->increment($key, $value);
	}

	function decrement($key, $value = 1) {
		return $this->_connection->decrement($key, $value);
	}
	
	function delete($key) {
		return $this->_connection->delete($key);
	}
	
	function clear() {
		$this->_connection->flush();
	}
}

/**
 * APC localized memory cache
 * @link	http://pecl.php.net/package/apc
 */
class ApcCacheStrategy extends AbstractCacheStrategy {
	function get($key) {
		if (is_array($key))
			return parent::get($key);
		return apc_fetch($key);
	}

	function set($key, $value, $ttl = null) {
		return apc_store($key, $value, _synd_cache_jitter($ttl));
	}
	
	function delete($key) {
		return apc_delete($key);
	}
	
	function clear() {
		apc_clear_cache();
		apc_clear_cache('user');
		apc_clear_cache('opcode');
	}
}

/**
 * XCache localized memory cache
 * @link	http://trac.lighttpd.net/xcache/
 */
class XCacheStrategy extends AbstractCacheStrategy {
	function get($key) {
		if (is_array($key))
			return parent::get($key);
		if (null !== ($value = xcache_get($key)))
			return $value;
		return false;
	}

	function set($key, $value, $ttl = null) {
		return xcache_set($key, $value, _synd_cache_jitter($ttl));
	}
	
	function delete($key) {
		return xcache_unset($key);
	}

	function add($key, $value, $ttl = null) {
		if (xcache_isset($key))
			return false;
		return xcache_set($key, $value, _synd_cache_jitter($ttl));
	}
	
	function replace($key, $value, $ttl = null) {
		if (xcache_isset($key))
			return xcache_set($key, $value, _synd_cache_jitter($ttl));
		return false;
	}

	function increment($key, $value = 1) {
		if (xcache_isset($key))
			return xcache_inc($key, $value);
		return false;
	}

	function decrement($key, $value = 1) {
		if (xcache_isset($key))
			return xcache_dec($key, $value);
		return false;
	}
	
	function clear() {}
}

/**
 * Filesystem based cache
 */
class FilesystemCacheStrategy extends AbstractCacheStrategy {
	/**
	 * @access	protected
	 */
	var $_path = null;
	
	function __construct($path) {
		$this->_path = $path;
	}
	
	function get($key) {
		if (is_array($key))
			return parent::get($key);
		if (file_exists($file = $this->_path.md5($key)) && 
			false !== ($value = file_get_contents($file)) && strlen($value) > 9 &&
			(0 === ($expire = hexdec(substr($value,0,8))) || $expire >= time()))
			return unserialize(substr($value, 8));
		return false;
	}

	function set($key, $value, $ttl = null) {
		if (SyndLib::createDirectory($this->_path) && 
			false !== ($fp = fopen($this->_path.md5($key), 'w')) &&
			false !== (fwrite($fp, null != $ttl ? dechex(time() + _synd_cache_jitter($ttl)) : '00000000')) &&
			false !== fwrite($fp, serialize($value)))
			return true;
		return false;
	}
	
	function delete($key) {
		return SyndLib::unlink($this->_path.md5($key));
	}
	
	function clear() {
		if (file_exists($this->_path))
			SyndLib::unlink($this->_path, true);
	}
}
