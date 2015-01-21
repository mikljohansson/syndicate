<?php
require_once 'core/Module.class.inc';
require_once 'core/model/lob/LobStorageDevice.class.inc';
require_once 'core/lib/rpc/RpcTransport.class.inc';
require_once 'core/lib/SyndLib.class.inc';

/**
 * LOB storage device
 *
 * Provides physical LOB storage. This module handles the physical 
 * file storage
 *
 * File storage can secured using mod_auth_token for Apache or 
 * mod_secdownload for LIGHTTPD who allows download authentication 
 * using tokens that may be expired to prevent deep-linking. 
 *
 * <code>
 * Apache httpd.conf example:
 *
 *   <Location /var/>
 *     AuthTokenSecret	"secret password"
 *     AuthTokenPrefix	/var/
 *     AuthTokenTimeout	3600
 *   </Location>
 *
 * LIGHTTPD mod_secdownload config example:
 *
 *   secdownload.secret        = "secret password"
 *   secdownload.document-root = "/home/info_synd/www/htdocs/var/"
 *   secdownload.uri-prefix    = "/var/"
 *   secdownload.timeout       = 3600
 *
 * local/synd.inc example:
 *
 * $synd_config = array(
 *   ...
 *   'device' => array(
 *     'path' => $_SERVER['DOCUMENT_ROOT'],
 *     'uri' => '/var/',
 *     'auth_token_secret' => 'secret password',
 *     ),
 *   ...
 *
 * </code>
 *
 * @see			LobStorageDevice
 * @access		protected
 * @package		synd.core.module
 * @link		http://www.synd.info/downloads/releases/
 * @link		http://www.lighttpd.net/documentation/secdownload.html
 */
class synd_module_device extends Module /* implements LobStorageDevice */ {
	function set($namespace, $variable, $uri, $checksum, $size) {
		if (null == ($target = $this->_path($namespace, $variable)) || !SyndLib::createDirectory(dirname($target))) {
			trigger_error("Failed to create target path for '$namespace:$variable'", E_USER_WARNING);
			return false;
		}
		
		set_time_limit(3600);
		$tmpfile = dirname($target)."/$checksum.tmp";

		if ($size > 0 && !copy($uri, $tmpfile) || !touch($tmpfile)) {
			trigger_error("Failed to copy '$uri' to '$tmpfile'", E_USER_WARNING);
			return false;
		}
		
		if (($sum = md5_file($tmpfile)) != $checksum) {
			trigger_error("Checksum verification for '$namespace:$variable' failed (expected '$checksum', got '$sum')", E_USER_WARNING);
			unlink($tmpfile);
			if (!file_exists($target))
				$this->delete($namespace, $variable);
			return false;
		}
		
		if (!$this->_replace($tmpfile, $target)) {
			unlink($tmpfile);
			return false;
		}
		
		return true;
	}

	function put($namespace, $variable, $buffer, $checksum) {
		if (null == ($target = $this->_path($namespace, $variable)) || !SyndLib::createDirectory(dirname($target))) {
			trigger_error("Failed to create target path for '$namespace:$variable'", E_USER_WARNING);
			return false;
		}
		
		set_time_limit(3600);
		$tmpfile = dirname($target)."/$checksum.tmp";

		if (!SyndLib::file_put_contents($tmpfile, $buffer)) {
			trigger_error("Failed to write buffer to '$target'", E_USER_WARNING);
			return false;
		}
		
		if (($sum = md5_file($tmpfile)) != $checksum) {
			trigger_error("Checksum verification for '$namespace:$variable' failed (expected '$checksum', got '$sum')", E_USER_WARNING);
			unlink($tmpfile);
			if (!file_exists($target))
				$this->delete($namespace, $variable);
			return false;
		}
		
		if (!$this->_replace($tmpfile, $target)) {
			unlink($tmpfile);
			return false;
		}
		
		return true;
	}
	
	/**
	 * @access	private
	 */
	function _replace($source, $target) {
		if (file_exists($target)) {
			if (!rename($target, "$target.bak"))
				return false;
			if (!rename($source, $target)) {
				rename("$target.bak", $target);
				return false;
			}
			unlink("$target.bak");
		}
		else if (!rename($source, $target))
			return false;
		return true;
	}		
	
	
	function replicate($namespace, $variable, $uri, $checksum, $size, $callback, $userarg = null) {
		$this->_task('replicate.'.$this->_hashns($namespace).'.'.$this->_hashkey($variable), 
			array($this, '_callback_replicate'), array($namespace, $variable, $uri, $checksum, $size, $callback, $userarg));
		return true;
	}
	
	/**
	 * LOB replication task
	 * @access	protected
	 */
	function _callback_replicate($namespace, $variable, $uri, $checksum, $size, $callback, $userarg) {
		if ($this->set($namespace, $variable, $uri, $checksum, $size))
			$callback[0]->$callback[1]($this, $namespace, $variable, $checksum, $userarg);
	}

	function get($namespace, $variable, $size = null) {
		if (null != ($path = $this->_path($namespace, $variable)) && file_exists($path) && (null === $size || filesize($path) == $size))
			return $this->_uri($namespace, $variable);
		return null;
	}
	
	function delete($namespace, $variable) {
		if (null == ($path = $this->_path($namespace, $variable))) 
			return false;
		if (!file_exists($path))
			return true;

		set_time_limit(3600);
		if (!unlink($path))
			return false;

		for ($d=dirname($path); trim($d,'/') != trim($this->_config['path'],'/') && @rmdir($d); $d=dirname($d));
		return true;
	}
	
	function checksum($namespace, $variables) {
		$result = array();
		foreach ($variables as $variable) {
			if (null != ($path = $this->_path($namespace, $variable)) && file_exists($path))
				$result[$variable] = md5_file($path);
		}
		return $result;
	}
	
	function getTotalSpace() {
		if (SyndLib::createDirectory($this->_config['path']))
			return disk_total_space($this->_config['path']);
		return 0;
	}

	function getFreeSpace() {
		if (SyndLib::createDirectory($this->_config['path']))
			return disk_free_space($this->_config['path']);
		return 0;
	}

	/**
	 * Appends the 'invoke' permission
	 */
	function getDefinedPermissions() {
		$perms = parent::getDefinedPermissions();
		$perms['invoke'] = SyndLib::translate('Allow complete RPC method invocation. Trackers need this permission to be able to communicate with this device.');
		return $perms;
	}
	
	function getDescription() {
		return SyndLib::translate('Provides physical LOB storage. This module handles the physical file storage.');
	}
	
	/**
	 * @access	protected
	 */
	function _hook_manage_permissions(&$result) {
		$result[] = $this;	
	}

	/**
	 * Absolute local path to LOB like "/var/apache/html/var/storage/test.txt"
	 * @access	protected
	 * @param	string		Namespace
	 * @param	string		Namespace unique key
	 * @return	string
	 */
	function _path($namespace, $variable) {
		return rtrim($this->_config['path'],'/').$this->_relative(
			$this->_hashns($namespace), $this->_hashkey($variable)).basename($variable);
	}

	/**
	 * Absolute URI to LOB like "http://node01.example.com/var/storage/test.txt"
	 * @access	protected
	 * @param	string		Namespace
	 * @param	string		Namespace unique key
	 * @return	string
	 */
	function _uri($namespace, $variable) {
		$host = isset($this->_config['host']) ? $this->_config['host'] : tpl_request_host();
		$prefix = rtrim($this->_config['uri'],'/');
		
		if (isset($this->_config['auth_token_secret'])) {
			$ts = dechex(time());
			$base = $this->_relative($this->_hashns($namespace), $this->_hashkey($variable), '/');
			$token = md5($this->_config['auth_token_secret'].$base.basename($variable).$ts);
			return $host.$prefix.'/'.$token.'/'.$ts.$base.rawurlencode(basename($variable));
		}

		return $host.$prefix.$this->_relative($this->_hashns($namespace), 
			$this->_hashkey($variable), '/').rawurlencode(basename($variable));
	}

	/**
	 * Relative path inside storage directory
	 * @access	protected
	 * @param	string	Namespace hash
	 * @param	string	LOB identifier hash
	 * @param	string	Directory separator
	 * @return	string
	 */
	function _relative($nshash, $keyhash, $sep = DIRECTORY_SEPARATOR) {
		return $sep.$nshash.$sep.
			substr($keyhash,0,2).$sep.
			substr($keyhash,2,4).$sep.
			substr($keyhash,6,4).$sep.
			substr($keyhash,10).$sep;
	}

	/**
	 * @access	private
	 */
	function _hashns($namespace) {
		return dechex(crc32($namespace));
	}

	/**
	 * @access	private
	 */
	function _hashkey($variable) {
		return md5($variable);
	}

	/**
	 * Schedule a task
	 * @access	private
	 */
	function _task($id, $callback, $args = null) {
		require_once 'core/lib/SyndTaskHandler.class.inc';
		$handler = SyndTaskHandler::factory('device');
		if (!$handler->isScheduled($id))
			$handler->appendTask(SyndType::factory('task', $id, $callback, $args), $id);
	}
}
