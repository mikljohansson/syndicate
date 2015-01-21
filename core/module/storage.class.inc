<?php
require_once 'core/Module.class.inc';
require_once 'core/lib/rpc/RpcTransport.class.inc';

/**
 * LOB storage client
 *
 * Provides the framework with clustered LOB storage access. 
 *
 * Uses time limited token-based authentication for securing 
 * downloads and prevent deep-linking. Coupled with Apache 
 * mod_auth_token downloads can be secured all through to the 
 * backend storage server.
 *
 * @access		protected
 * @package		synd.core.module
 */
class synd_module_storage extends Module {
	/**
	 * Limit in bytes of size to transfer inline
	 * @access	private
	 */
	var $_inline = 4096;
	
	/**
	 * @access	private
	 */
	var $_key = null;

	function __construct($config = null) {
		parent::__construct($config);

		if (empty($config['replicas']))
			$config['replicas'] = 2;
		
		SyndLib::attachHook('lob_storage_set', array($this, '_callback_lob_storage_set'));
		SyndLib::attachHook('lob_storage_put', array($this, '_callback_lob_storage_put'));
		SyndLib::attachHook('lob_storage_copy', array($this, '_callback_lob_storage_copy'));
		SyndLib::attachHook('lob_storage_uri', array($this, '_callback_lob_storage_uri'));
		SyndLib::attachHook('lob_storage_delete', array($this, '_callback_lob_storage_delete'));
		SyndLib::attachHook('lob_storage_stat', array($this, '_callback_lob_storage_stat'));
	}

	function _callback_lob_storage_set(&$result, $variable, $file, $replicas = null) {
		if (false === ($size = filesize($file)))
			return false;
		$result = $size > $this->_inline ?
			$this->_set($variable, $file, $size, null !== $replicas ? $replicas : $this->_config['replicas']) :
			$this->_put($variable, file_get_contents($file), $size, null !== $replicas ? $replicas : $this->_config['replicas']);
		return true;
	}
	
	function _callback_lob_storage_put(&$result, $variable, $buffer, $replicas = null) {
		return $result = $this->_put($variable, $buffer, strlen($buffer), 
			null !== $replicas ? $replicas : $this->_config['replicas']);
	}

	/**
	 * @access	private
	 */
	function _set($variable, $file, $size, $replicas) {
		global $synd_config;
		set_time_limit(3600);

		$result = false;
		$hash = md5($this->_config['namespace'].'.'.$variable);
		$path = $synd_config['dirs']['var']['path']."tmp/$hash.lob";
		
		$host = isset($this->_config['host']) ? $this->_config['host'] : tpl_request_host();
		$uri = $host.$synd_config['dirs']['var']['uri']."tmp/$hash.lob";
		
		if (!SyndLib::createDirectory(dirname($path)))
			return false;
		
		if (symlink($file, $path) || copy($file, $path)) {
			$checksum = md5_file($file);
			foreach ($this->getTrackerUrns() as $urn) {
				if (null !== ($tracker = Activator::getInstance($urn)) && 
					null != ($result = $tracker->set($this->_config['namespace'], trim($variable,'/'), $uri, $checksum, $size, $replicas)))
					break;
			}
		}
		
		unlink($path);
		return $result ? tpl_request_host().tpl_view('storage').trim($variable,'/') : false;
	}
	
	/**
	 * @access	private
	 */
	function _put($variable, $buffer, $size, $replicas) {
		set_time_limit(3600);

		$result = false;
		$checksum = md5($buffer);
		
		foreach ($this->getTrackerUrns() as $urn) {
			if (null !== ($tracker = Activator::getInstance($urn)) && 
				null != ($result = $tracker->put($this->_config['namespace'], trim($variable,'/'), $buffer, $checksum, $size, $replicas)))
				break;
		}

		return $result ? tpl_request_host().tpl_view('storage').trim($variable,'/') : false;
	}

	/**
	 * @access	private
	 */
	function _get($variable) {
		foreach ($this->getTrackerUrns() as $urn) {
			if (null !== ($tracker = Activator::getInstance($urn)) && 
				false !== ($uri = $tracker->get($this->_config['namespace'], trim($variable,'/'))))
				return $uri;
		}
		return false;
	}

	function _callback_lob_storage_copy(&$result, $source, $target) {
		set_time_limit(3600);
		$result = false;

		foreach ($this->getTrackerUrns() as $urn) {
			if (null !== ($tracker = Activator::getInstance($urn)) && 
				null != ($result = $tracker->copy($this->_config['namespace'], trim($source,'/'), $this->_config['namespace'], trim($target,'/'))))
				break;
		}

		return $result ? tpl_request_host().tpl_view('storage').trim($target,'/') : false;
	}

	/**
	 * Returns the URI of a LOB
	 * @param	string	Name of LOB
	 * @param	bool	Check for LOB existance
	 * @param	bool	Return absolute URI to device storing LOB
	 * @return	string
	 */
	function _callback_lob_storage_uri(&$result, $variable, $stat = false, $dereference = false) {
		if ($dereference)
			$result = $this->_get($variable);
		else if (!$stat || null != $this->_get($variable)) {
			$timestamp = dechex(time());
			$relative = '/'.implode('/', array_map('rawurlencode', explode('/', trim($variable,'/'))));
			$token = md5($this->getPrivateKey().'/'.trim($variable,'/').$timestamp);
			$result = tpl_request_host().tpl_view('storage').$token.'/'.$timestamp.$relative;
		}
		debug($variable,$result);
		return true;
	}

	function _callback_lob_storage_delete(&$result, $variable, $recursive = false) {
		foreach ($this->getTrackerUrns() as $urn) {
			if (null !== ($tracker = Activator::getInstance($urn)) && 
				$tracker->delete($this->_config['namespace'], trim($variable,'/'), $recursive)) {
				$result = true;
				break;
			}
		}

		return true;
	}
	
	function _callback_lob_storage_stat(&$result, $variable) {
		foreach ($this->getTrackerUrns() as $urn) {
			if (null !== ($tracker = Activator::getInstance($urn)) && 
				false !== ($result = $tracker->stat($this->_config['namespace'], trim($variable,'/')))) {
				break;
			}
		}

		return true;
	}

	/**
	 * Finds a device using the tracker and redirects to the real LOB location
	 * 
	 * Path is encoded as 
	 *  array(token, timestamp, variable)
	 */
	function request(Request $request) {
		$path = $request->getContext();
		if (empty($path[0]) || empty($path[1]) || count($path) < 3)
			throw new InvalidArgumentException();
		
		// Check token time to live
		$ttl = isset($this->_config['ttl']) ? $this->_config['ttl'] : 604800;
		if (time() > hexdec($path[1]) + $ttl)
			throw new ForbiddenException(SyndLib::translate('This download has expired, try pressing back and reloading the page'));
		
		// Validate user supplied token
		$variable = '/'.implode('/', array_slice($path, 2));
		$token = md5($this->getPrivateKey().$variable.$path[1]);
		if (strtolower($token) != strtolower($path[0]))
			throw new ForbiddenException();
		
		// Redirect to variable URI at storage device
		if (null != ($uri = $this->_get($variable)))
			return array('redirect' => $uri);
		throw new NotFoundException();
	}

	/**
	 * Returns an instance from the list of available trackers
	 * @return	LobStorageTracker	Return null on failure
	 */
	function getTracker() {
		$tracker = null;
		foreach ($this->getTrackerUrns() as $urn) {
			if (null !== ($tracker = Activator::getInstance($urn)))
				break;
		}
		return $tracker;
	}
	
	/**
	 * Returns the configured trackers URNs
	 * @return	array
	 */
	function getTrackerUrns() {
		$trackers = $this->_config['trackers'];
		shuffle($trackers);
		return $trackers;
	}
	
	/**
	 * Returns the configured namespace
	 * @return	string
	 */
	function getNamespace() {
		return $this->_config['namespace'];
	}

	/**
	 * Returns a authentication token
	 * @param	Relative path to authenticate
	 * @return	string
	 */
	function getPrivateKey() {
		if (null === $this->_key && null == ($this->_key = SyndLib::runHook('variable_get', 'storage', 'private_key')))
			SyndLib::runHook('variable_set', 'storage', 'private_key', $this->_key = md5(uniqid(rand(), true)));
		return $this->_key;
	}
}
