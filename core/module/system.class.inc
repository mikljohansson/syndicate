<?php
require_once 'core/Module.class.inc';

/**
 * System administration module
 * @access		protected
 * @package		synd.core.module
 */
class synd_module_system extends Module {
	function __construct($config = null) {
		if (!isset($config['storage']['namespace']))
			$config['storage']['namespace'] = $_SERVER['SERVER_NAME'];
		
		parent::__construct($config);

		if (!isset($_SESSION['synd']['timezone_offset']))
			SyndLib::attachHook('html_head', array($this, '_callback_html_head'));
		SyndLib::attachHook('menu', array($this, '_callback_menu'), -250);

		if (isset($config['storage']['path'], $config['storage']['uri'])) {
			SyndLib::attachHook('lob_storage_set', array($this, '_callback_lob_storage_set'), -250);
			SyndLib::attachHook('lob_storage_put', array($this, '_callback_lob_storage_put'), -250);
			SyndLib::attachHook('lob_storage_copy', array($this, '_callback_lob_storage_copy'), -250);
			SyndLib::attachHook('lob_storage_uri', array($this, '_callback_lob_storage_uri'), -250);
			SyndLib::attachHook('lob_storage_delete', array($this, '_callback_lob_storage_delete'), -250);
			SyndLib::attachHook('lob_storage_stat', array($this, '_callback_lob_storage_stat'), -250);
		}

		SyndLib::attachHook('variable_set', array($this, '_callback_variable_set'));
		SyndLib::attachHook('variable_get', array($this, '_callback_variable_get'));
		SyndLib::attachHook('default_request_handler', array($this, '_callback_default_request_handler'), -250);
	}
	
	/**
	 * @access	protected
	 */
	function _hook_manage_permissions(&$result) {
		$result[] = $this;	
	}

	function getDescription() {
		return SyndLib::translate('System administration module');
	}
	
	function _callback_menu(&$result, $menu) {
		SyndLib::runHook('menu_admin', $indicator = new IndicatorMenu());
		if ($indicator->hasChildren())
			$menu->append(tpl_link('system','admin'), SyndLib::translate('Administer'), '', array($this, '_callback_menu_default'), array('system/*','system'));
	}
	
	function _callback_menu_default($menu) {
		SyndLib::runHook('menu_admin', $menu);
	}

	function _callback_html_head(&$result, Template $page) {
		$result .= $page->fetch(tpl_design_path('module/system/timezone.tpl'));
	}

	function _callback_lob_storage_set(&$result, $variable, $file, $replicas = null) {
		$path = $this->_getAbsolutePath($variable);
		if (SyndLib::createDirectory(dirname($path)) && copy($file, $path)) 
			return $result = $this->_getAbsoluteUri($variable);
		return false;
	}
	
	function _callback_lob_storage_put(&$result, $variable, $buffer, $replicas = null) {
		$path = $this->_getAbsolutePath($variable);
		if (SyndLib::createDirectory(dirname($path)) && SyndLib::file_put_contents($path, $buffer))
			return $result = $this->_getAbsoluteUri($variable);
		return false;
	}

	function _callback_lob_storage_copy(&$result, $source, $target) {
		$path = $this->_getAbsolutePath($target);
		if (SyndLib::createDirectory(dirname($path)) && copy($this->_getAbsolutePath($source), $path)) 
			return $result = $this->_getAbsoluteUri($target);
		return false;
	}

	function _callback_lob_storage_uri(&$result, $variable, $stat = false, $dereference = false) {
		if (!$stat || file_exists($this->_getAbsolutePath($variable)))
			$result = $this->_getAbsoluteUri($variable);
		return true;
	}

	function _callback_lob_storage_delete(&$result, $variable, $recursive = false) {
		return $result = SyndLib::unlink($this->_getAbsolutePath($variable), $recursive);
	}
	
	function _callback_lob_storage_stat(&$result, $variable) {}
	
        /**
         * Absolute local path to LOB like "/var/apache/html/var/storage/test.txt"
         * @access      protected
         * @param       string          Namespace unique key
         * @return      string
         */
        function _path($variable) {
		$namespace = $this->_config['storage']['namespace'];
                return rtrim($this->_config['storage']['path'],'/').$this->_relative(
                        $this->_hashns($namespace), $this->_hashkey($variable)).basename_safe($variable);
        }

        /**
         * Absolute URI to LOB like "http://node01.example.com/var/storage/test.txt"
         * @access      protected
         * @param       string          Namespace
         * @param       string          Namespace unique key
         * @return      string
         */
        function _uri($variable) {
		$namespace = $this->_config['storage']['namespace'];
                $host = tpl_request_host();
                $prefix = rtrim($this->_config['storage']['uri'],'/');

                if (isset($this->_config['storage']['auth_token_secret'])) {
                        $ts = dechex(time());
                        $base = $this->_relative($this->_hashns($namespace), $this->_hashkey($variable), '/');
                        $token = md5($this->_config['storage']['auth_token_secret'].$base.basename_safe($variable).$ts);
                        return $host.$prefix.'/'.$token.'/'.$ts.$base.rawurlencode(basename_safe($variable));
                }

                return $host.$prefix.$this->_relative($this->_hashns($namespace),
                        $this->_hashkey($variable), '/').rawurlencode(basename_safe($variable));
        }

        /**
         * Relative path inside storage directory
         * @access      protected
         * @param       string  Namespace hash
         * @param       string  LOB identifier hash
         * @param       string  Directory separator
         * @return      string
         */
        function _relative($nshash, $keyhash, $sep = DIRECTORY_SEPARATOR) {
                return $sep.$nshash.$sep.
                        substr($keyhash,0,2).$sep.
                        substr($keyhash,2,4).$sep.
                        substr($keyhash,6,4).$sep.
                        substr($keyhash,10).$sep;
        }

        /**
         * @access      private
         */
        function _hashns($namespace) {
                return dechex(crc32($namespace));
        }

        /**
         * @access      private
         */
        function _hashkey($variable) {
                return md5($variable);
        }

	/**
	 * @access	private
	 */
	function _getAbsolutePath($variable) {
		return $this->_path($variable);
		
		return rtrim($this->_config['storage']['path'],DIRECTORY_SEPARATOR).DIRECTORY_SEPARATOR.$variable;
	}
	
	/**
	 * @access	private
	 */
	function _getAbsoluteUri($variable) {
		return $this->_uri($variable);

		$base = rtrim($this->_config['storage']['uri'],'/').'/';

		if (isset($this->_config['storage']['auth_token_secret'])) {
			$timestamp = dechex(time());
			$token = md5($this->_config['storage']['auth_token_secret'].'/'.trim($variable,'/').$timestamp);
			$base .= $token.'/'.$timestamp.'/';
		}
		
		$relative = implode('/', array_map('rawurlencode', explode('/', trim($variable,'/'))));
		return tpl_request_host().$base.$relative;
	}
	
	function _callback_variable_set(&$result, $namespace, $variable, $value, $ttl = null) {
		global $synd_maindb;
		assert('strlen($namespace) <= 64 && strlen($variable) <= 64');
		
		if (null === $value) {
			$sql = "
				DELETE FROM synd_variable
				WHERE 
					namespace = ".$synd_maindb->quote($namespace)." AND
					variable = ".$synd_maindb->quote($variable);
			$result = !SyndLib::isError($synd_maindb->query($sql));
		}
		else {
			$data = array(
				'NAMESPACE'	=> $namespace,
				'VARIABLE'	=> $variable,
				'EXPIRES'	=> null !== $ttl ? time() + $ttl : null,
				'VALUE'		=> $synd_maindb->wrapBinary(serialize($value)));
			$result = !SyndLib::isError($synd_maindb->replace('synd_variable', $data));
		}
		
		if (rand(1,isset($this->_config['gc_divisor']) ? $this->_config['gc_divisor'] : 1000) <= (
			isset($this->_config['gc_probability']) ? $this->_config['gc_probability'] : 1)) {
			$sql = "
				DELETE FROM synd_variable
				WHERE expires < ".$synd_maindb->quote(time());
			$synd_maindb->query($sql);		
		}
	}
	
	function _callback_variable_get(&$result, $namespace, $variable) {
		global $synd_maindb;
		$sql = "
			SELECT v.value FROM synd_variable v
			WHERE 
				v.namespace = ".$synd_maindb->quote($namespace)." AND
				v.variable = ".$synd_maindb->quote($variable)." AND
				(v.expires IS NULL OR v.expires >= ".$synd_maindb->quote(time()).")";
		if (null !== ($buffer = $synd_maindb->getOne($sql))) {
			$result = unserialize($synd_maindb->unwrapBinary($buffer));
			return true;
		}
	}
	
	function _callback_default_request_handler(&$result) {
		$result = $this;
		return true;
	}
	
	function _view_index(Request $request, Template $page) {
		return array('content' => $page->fetch(tpl_design_path('module/system/index.tpl')));
	}
	
	function _view_admin(Request $request, Template $page) {
		return array('content' => $page->fetch(tpl_design_path('module/system/admin.tpl')));
	}
	
	function _view_setTimezone(Request $request, Template $page) {
		$_SESSION['synd']['timezone_offset'] = isset($request['zone']) && 
			is_numeric($request['zone']) && 0 == $request['zone']%60 ? $request['zone'] : 0;
		return array('content' => '', 'page' => null);
	}

	function _view_setEnvironment(Request $request, Template $page) {
		global $synd_user;
		if (isset($request['senv'])) {
			if (!isset($_SESSION['synd']['public'])) 
				$_SESSION['synd']['public'] = $request['senv'];
			else {
				$_SESSION['synd']['public'] = SyndLib::array_merge_recursive(
					$_SESSION['synd']['public'], $request['senv'], false);
			}
		}
	}

	/**
	 * Stub for using the mplex system
	 */
	function _view_mplex() {}
	
	/**
	 * Invokes a method on an instance
	 *
	 * Method should have the prototype "mres = _view_{$method}(array $request)"
	 * and will have to do its own access control and data validation.
	 */
	function _view_invoke(Request $request, Template $page) {
		if (!isset($request[0]) || !isset($request[1]))
			return null;
		if (null === ($instance = SyndLib::getInstance($request[0])))
			throw new NotFoundException();
		
		$function = array($instance, '_view_'.$request[1]);
		if (!is_callable($function))
			throw new NotFoundException();

		return call_user_func($function, $request, $page);
	}
}
