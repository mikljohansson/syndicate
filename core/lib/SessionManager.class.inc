<?php

/**
 * Abstract session handler
 *
 * <code>
 * // Register distributed session handler
 * require_once 'core/lib/SessionManager.class.inc';
 * require_once 'core/lib/CacheStrategy.class.inc';
 * require_once 'core/db/Database.class.inc';
 *
 * $dsn = 'cluster://mysql://master.example.com/db;mysql://slave1.example.com/db,mysql://slave2.example.com/db';
 *
 * SessionManager::register(
 *     new BlockingSessionManager(new CachedSessionManager(
 *     new DatabaseSessionManager(DatabaseManager::getConnection($dsn)), 
 *     CacheStrategyManager::factory())));
 * </code>
 *
 * @access		public
 * @package		synd.core.lib
 */
abstract class SessionManager implements SessionHandlerInterface {
	/**
	 * Register a session handler
	 * @param	SessionManager	The session handler to use
	 */
	static function register($handler) {
		if (!session_set_save_handler($handler, true)) {
			throw new RuntimeException('Failed to register session handler');
		}
	}

	/**
	 * Return a new session ID
	 * @return	string	A session ID valid for the default session handler.
	 */
	function create_sid() {
		return md5(uniqid(''));
	}

	
	/**
	 * Opens the session
	 * @param	string	Path to store sessions (from php.ini)
	 * @param	string	The session id
	 * @return	bool	Returns true on success, false on failure.
	 */
	abstract function open($path, $id);

	/**
	 * Close the session
	 * @return	bool	Returns true on success, false on failure.
	 */
	abstract function close();
	
	/**
	 * Read session data
	 * @param	string	The session id
	 * @return	string	Returns empty string if no data, false on failure.
	 */
	abstract function read($id);
	
	/**
	 * Write session data
	 * @param	string	The session id
	 * @param	string	Data to write
	 * @return	bool	Returns true on success, false on failure.
	 */
	abstract function write($id, $content);
	
	/**
	 * Delete a session
	 * @param	string	The session id
	 * @return	bool	Returns true on success, false on failure.
	 */
	abstract function destroy($id);
	
	/**
	 * Garbage collect session
	 * @param	integer	Time to live in seconds
	 * @return	bool	Returns true on success, false on failure.
	 */
	abstract function gc($ttl);
}

/**
 * Database session handler
 */
class DatabaseSessionManager extends SessionManager {
	/**
	 * @access	protected
	 */
	var $_db = null;
	
	/**
	 * @access	protected
	 */
	var $_refresh = null;
	
	/**
	 * The database needs the 'synd_session' table
	 * @param	Database	Database to store session
	 */
	function __construct(Database $db) {
		$this->_db = $db;
	}
	
	function open($path, $id) {
		return true;
	}
	
	function close() {
		if (!empty($this->_refresh)) {
			try {
				$sql = "
					UPDATE synd_session SET refreshed = ".time()."
					WHERE sid IN (".implode(',',$this->_db->quote($this->_refresh)).")";
				$this->_db->query($sql);
				return true;
			}
			catch (DatabaseException $e) {
				trigger_error($e->getMessage(), E_USER_WARNING);
				return false;
			}
		}
	}
	
	function read($id) {
		try {
			$sql = "
				SELECT s.REFRESHED, s.CONTENT FROM synd_session s
				WHERE s.sid = ".$this->_db->quote($id);
			if (null == ($row = $this->_db->getRow($sql)))
				return '';
			
			$ts = time();
			$lt = ini_get('session.gc_maxlifetime');
			if ($row['REFRESHED'] < ($ts - $lt))
				return '';
			if ($row['REFRESHED'] < ($ts - (integer)($lt*0.2)))
				$this->_refresh[] = $id;
			return $this->_db->unwrapBinary($row['CONTENT']);
		}
		catch (DatabaseException $e) {
			trigger_error($e->getMessage(), E_USER_WARNING);
			return false;
		}
	}
	
	function write($id, $content) {
		try {
			if (null !== $this->_refresh && false !== ($key = array_search($id, $this->_refresh)))
				unset($this->_refresh[$key]);
			$this->_db->replace('synd_session', array(
				'SID' => $id,
				'REFRESHED' => time(),
				'CONTENT' => $this->_db->wrapBinary($content)));
			return true;
		}
		catch (DatabaseException $e) {
			trigger_error($e->getMessage(), E_USER_WARNING);
			return false;
		}
	}
	
	function destroy($id) {
		try {		
			$sql = "
				DELETE FROM synd_session
				WHERE sid = ".$this->_db->quote($id);
			$this->_db->query($sql);
			return true;
		}
		catch (DatabaseException $e) {
			trigger_error($e->getMessage(), E_USER_WARNING);
			return false;
		}
	}
	
	function gc($ttl) {
		try {		
			$sql = "
				DELETE FROM synd_session
				WHERE refreshed < ".(time()-$ttl);
			$this->_db->query($sql);
			return true;
		}
		catch (DatabaseException $e) {
			trigger_error($e->getMessage(), E_USER_WARNING);
			return false;
		}
	}
}

/**
 * Cached session handler
 */
class CachedSessionManager extends SessionManager {
	protected $_next = null;
	protected $_strategy = null;
	
	/**
	 * @see		CacheStrategyManager::factory()
	 * @param	SessionManager		Next in chain
	 * @param	CacheStrategy		Caching strategy
	 */
	function __construct(SessionManager $next, CacheStrategy $strategy) {
		$this->_next = $next;
		$this->_strategy = $strategy;
	}
	
	function open($path, $id) {
		return $this->_next->open($path, $id);
	}
	
	function close() {
		return $this->_next->close();
	}
	
	function read($id) {
		if (false == ($content = $this->_strategy->get('session.'.$id)) && false != ($content = $this->_next->read($id)))
			$this->_strategy->set('session.'.$id, $content, (integer)(ini_get('session.gc_maxlifetime')*0.25));
		return $content;
	}
	
	function write($id, $content) {
		$this->_strategy->set('session.'.$id, $content, (integer)(ini_get('session.gc_maxlifetime')*0.25));
		return $this->_next->write($id, $content);
	}
	
	function destroy($id) {
		$this->_strategy->delete('session.'.$id);
		return $this->_next->destroy($id);
	}
	
	function gc($ttl) {
		return $this->_next->gc($ttl);
	}
}

/**
 * Checksums session and blocks unnessesary writes
 *
 * This requires persistent handlers down the chain to be able to
 * refresh their expiry times using only the get() callback.
 */
class BlockingSessionManager extends SessionManager {
	protected $_next = null;
	protected $_checksums = null;

	/**
	 * @param	SessionManager		Next in chain
	 */
	function __construct(SessionManager $next) {
		$this->_next = $next;
	}

	function open($path, $id) {
		return $this->_next->open($path, $id);
	}
	
	function close() {
		return $this->_next->close();
	}
	
	function read($id) {
		if (false != ($content = $this->_next->read($id)))
			$this->_checksums[$id] = crc32($content);
		return $content;
	}
	
	function write($id, $content) {
		if (isset($this->_checksums[$id]) && crc32($content) != $this->_checksums[$id] || !isset($this->_checksums[$id]) && false != $content) {
			$this->_checksums[$id] = crc32($content);
			return $this->_next->write($id, $content);
		}
		return true;
	}

	function destroy($id) {
		if (isset($this->_checksums[$id]))
			unset($this->_checksums[$id]);
		return $this->_next->destroy($id);
	}
	
	function gc($ttl) {
		return $this->_next->gc($ttl);
	}
}
