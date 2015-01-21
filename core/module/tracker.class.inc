<?php
require_once 'core/Module.class.inc';
require_once 'core/model/lob/LobStorageTracker.class.inc';
require_once 'core/lib/rpc/RpcTransport.class.inc';

define('S_IFREG', 0x8000);	// regular file
define('S_IRUSR', 0x0100);	// owner has read permission
define('S_IWUSR', 0x0080);	// owner has write permission
define('S_IRGRP', 0x0020);	// group has read permission
define('S_IROTH', 0x0004);	// others have read permission

/**
 * LOB storage tracker
 *
 * Coordinates a cluster of LOB storage devices, internal 
 * communication is done over some form of RPC transport.
 *
 * Example of storage devices urns (given to synd_module_tracker)
 *
 *  php;https://com_example_www:secret@storage01.example.com/synd/rpc/device/
 *  php;https://com_example_www:secret@storage02.example.com/synd/rpc/device/
 *
 * Example of tracker urns (given to clients such as synd_module_storage)
 *
 *  php;https://com_example_www:secret@tracker01.example.com/synd/rpc/tracker/
 *  php;https://com_example_www:secret@tracker02.example.com/synd/rpc/tracker/
 *
 * It is recommended to use the PHP formatter and avoid the XML-RPC 
 * and SOAP implementation since they have a history of not being 
 * very binary safe, errorous data is detected by checksumming and 
 * will fail the transaction.
 *
 * @see			LobStorageTracker
 * @access		protected
 * @package		synd.core.module
 */
class synd_module_tracker extends Module /* implements LobStorageTracker */ {
	/**
	 * @access	protected
	 */
	var $_db = null;
	
	/**
	 * Retry failed storage interval in seconds
	 * @access	protected
	 */
	var $_retryInterval = 1;
	
	function __construct($config = null) {
		global $synd_maindb;
		parent::__construct($config);
		$this->_db = $synd_maindb;
		SyndLib::attachHook('menu_admin', array($this, '_callback_menu_admin'));
	}
	
	function set($namespace, $variable, $uri, $checksum, $size, $replicas) {
		if (null == ($nsid = $this->_nsid($namespace)) || null == $checksum || $replicas < 1)
			return false;
		
		$i = 0;
		$urns = $this->_getTargetDevices($nsid);
		set_time_limit(3600);
		
		foreach ($urns as $devid => $urn) {
			$this->_db->begin();

			// Lock single LOB row using UPDATE/INSERT
			if (null !== ($device = Activator::getInstance($urn, true, true)) &&
				null !== ($lobid = $this->_add($nsid, $variable, $checksum, $size, $replicas)) &&
				$this->_addReplica($devid, $lobid, $checksum)) {
				
				// Store LOB syncronously
				if ($device->set($namespace, $variable, $uri, $checksum, $size)) {
					$this->_db->commit();
					if ($replicas > 1 && count($urns) - $i > 1)
						$this->_task('replicate', array($this, '_callback_replicate'));
					return true;
				}
				// Check if target device failed
				else if (false === $device->get($namespace, $variable)) {
					$this->_failDevice($devid);
				}
			}
			
			$this->_db->rollback();
			$i++;
		}
		
		return false;
	}
	
	function put($namespace, $variable, $buffer, $checksum, $size, $replicas) {
		if (null == ($nsid = $this->_nsid($namespace)) || null == $checksum || $replicas < 1)
			return false;
		
		$i = 0;
		$urns = $this->_getTargetDevices($nsid);
		set_time_limit(3600);
		
		foreach ($urns as $devid => $urn) {
			$this->_db->begin();

			// Lock single LOB row using UPDATE/INSERT
			if (null !== ($device = Activator::getInstance($urn, true, true)) &&
				null !== ($lobid = $this->_add($nsid, $variable, $checksum, $size, $replicas)) &&
				$this->_addReplica($devid, $lobid, $checksum)) {
				
				// Store LOB syncronously
				if ($device->put($namespace, $variable, $buffer, $checksum)) {
					$this->_db->commit();
					if ($replicas > 1 && count($urns) - $i > 1)
						$this->_task('replicate', array($this, '_callback_replicate'));
					return true;
				}
				// Check if target device failed
				else if (false === $device->get($namespace, $variable)) {
					$this->_failDevice($devid);
				}
			}
			
			$this->_db->rollback();
			$i++;
		}
		
		return false;
	}
	
	function copy($sourcens, $sourcevar, $targetns, $targetvar) {
		if (null == ($nsid = $this->_nsid($targetns)))
			return false;
		
		// Find LOB meta information
		$sql = "
			SELECT l.CHECKSUM, l.SPACE, l.REPLICAS
			FROM synd_storage_namespace n, synd_storage_lob l
			WHERE
				n.nsid = l.nsid AND
				n.namespace = ".$this->_db->quote($sourcens)." AND
				l.variable = ".$this->_db->quote($sourcevar)." AND
				l.deleted = 0";
		if (null == ($meta = $this->_db->getRow($sql)))
			return false;
		
		$i = 0;
		$urns = $this->_getTargetDevices($nsid);
		$devices = $this->getDevices($sourcens, $sourcevar);
		set_time_limit(3600);

		if (empty($devices)) {
			trigger_error("No source replicas found for '$sourcens:$sourcevar'", E_USER_WARNING);
			return false;
		}
		
		foreach ($urns as $devid => $urn) {
			$this->_db->begin();

			// Lock single LOB row using UPDATE/INSERT
			if (null !== ($device = Activator::getInstance($urn, true, true)) &&
				null !== ($lobid = $this->_add($nsid, $targetvar, $meta['CHECKSUM'], $meta['SPACE'], $meta['REPLICAS'])) &&
				$this->_addReplica($devid, $lobid, $meta['CHECKSUM'])) {
				
				// Find a suitable source device
				foreach (array_keys($devices) as $srcdevid) {
					if (false === ($uri = $devices[$srcdevid]->get($sourcens, $sourcevar, $meta['SPACE'])))
						$this->_failDevice($srcdevid);
					else if (null == $uri)
						$this->_failReplica($srcdevid);
					else
						break;
				}
				
				// Copy LOB syncronously
				if ($device->set($targetns, $targetvar, $uri, $meta['CHECKSUM'], $meta['SPACE'])) {
					$this->_db->commit();
					if ($meta['REPLICAS'] > 1 && count($urns) - $i > 1)
						$this->_task('replicate', array($this, '_callback_replicate'));
					return true;
				}
				// Check if source device failed
				else if (false === ($uri = $devices[$srcdevid]->get($sourcens, $sourcevar, $meta['SPACE']))) {
					$this->_failDevice($srcdevid);
					unset($devices[$srcdevid]);
				}
				// Check if source replica missing
				else if (null == $uri) {
					$this->_failReplica($srcdevid);
					unset($devices[$srcdevid]);
				}
				// Target device failed
				else {
					$this->_failDevice($devid);
				}
			}

			$this->_db->rollback();
			$i++;
		}
		
		return false;
	}

	/**
	 * Returns suitable target devices
	 * @access	private
	 * @return	array	List of URNs indexed by devid
	 */
	function _getTargetDevices($nsid) {
		$sql = "
			SELECT d.DEVID, d.URN FROM synd_storage_device d
			WHERE 
				d.nsid = ".$this->_db->quote($nsid)." AND
				d.failed < ".(time()-$this->_retryInterval)."
			ORDER BY (d.space_total - d.space_used) * (1 - ".$this->_db->rand()." / 5) DESC";
		return (array)$this->_db->getAssoc($sql);
	}
	
	function get($namespace, $variable) {
		foreach ($this->getReplicas($namespace, $variable) as $replica) {
			if (null === ($device = Activator::getInstance($replica['URN'], true, true)))
				$this->_failDevice($replica['DEVID']);
			else if (false === ($uri = $device->get($namespace, $variable, $replica['SPACE'])))
				$this->_failDevice($replica['DEVID']);
			else if (null == $uri)
				$this->_failReplica($replica['DEVID']);
			else
				return $uri;
		}
		return false;
	}
	
	/**
	 * Returns all active URIs for a LOB
	 * @param	string	Namespace
	 * @param	string	Namespace unique key
	 * @return	array	List of URIs indexed by the device id they reside on
	 */
	function getLocations($namespace, $variable) {
		$result = array();
		foreach ($this->getReplicas($namespace, $variable) as $replica) {
			if (null === ($device = Activator::getInstance($replica['URN'], true, true)))
				$this->_failDevice($replica['DEVID']);
			else if (false === ($uri = $device->get($namespace, $variable, $replica['SPACE'])))
				$this->_failDevice($replica['DEVID']);
			else if (null == $uri)
				$this->_failReplica($replica['DEVID']);
			else
				$result[$replica['DEVID']] = $uri;
		}
		return $result;
	}

	function delete($namespace, $variable, $recursive = false) {
		$sql = "
			UPDATE synd_storage_lob SET deleted = 1
			WHERE
				deleted = 0 AND
				nsid = (
					SELECT n.nsid FROM synd_storage_namespace n
					WHERE n.namespace = ".$this->_db->quote($namespace).") AND
				";
		
		if ($recursive && null != $variable)
			$sql .= "variable LIKE ".$this->_db->quote($variable.'%');
		else
			$sql .= "variable = ".$this->_db->quote($variable);
		
		$result = $this->_db->query($sql);
		if (SyndLib::isError($result))
			return false;
		
		if ($this->_db->affectedRows())
			$this->_task('garbage_collect_lobs', array($this, '_callback_garbage_collect_lobs'));
		
		return true;
	}
	
	function stat($namespace, $variable) {
		$sql = "
			SELECT l.NSID, l.LOBID, l.SPACE, l.CREATED, l.MODIFIED
			FROM synd_storage_namespace n, synd_storage_lob l
			WHERE
				n.nsid = l.nsid AND
				n.namespace = ".$this->_db->quote($namespace)." AND
				l.variable = ".$this->_db->quote($variable);
		if (null == ($row = $this->_db->getRow($sql)))
			return null;
		
		return array(
			'dev'		=> (int)$row['NSID'],
			'ino'		=> (float)$row['LOBID'],
			'mode'		=> S_IFREG|S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH,	// 0644
			'nlink'		=> 1,
			'uid'		=> function_exists('posix_geteuid') ? posix_geteuid() : 0,
			'gid'		=> function_exists('posix_getegid') ? posix_getegid() : 0,
			'rdev'		=> 0,
			'size'		=> (float)$row['SPACE'],
			'atime'		=> (int)$row['MODIFIED'],
			'mtime'		=> (int)$row['MODIFIED'],
			'ctime'		=> (int)$row['CREATED'],
			'blksize'	=> -1,
			'blocks'	=> 1,
			);
	}

	/**
	 * Appends the 'invoke' permission
	 */
	function getDefinedPermissions() {
		$perms = parent::getDefinedPermissions();
		$perms['invoke'] = SyndLib::translate('Allow complete RPC method invocation. Clients need this permission to be able to communicate with this tracker.');
		return $perms;
	}

	function getDescription() {
		return SyndLib::translate('Coordinates a cluster of LOB storage devices and provides client access to the storage cluster.');
	}
	
	/**
	 * @access	protected
	 */
	function _hook_manage_permissions(&$result) {
		$result[] = $this;	
	}

	/**
	 * Replication task
	 *
	 * Will loop over all LOBs lacking enough replicas and schedule a 
	 * replication task on a suitable device
	 *
	 * @access	protected
	 */
	function _callback_replicate() {
		sleep(5);

		for ($offset=0; $offset < 25000; $offset+=1000) {
			// Find LOBs lacking enough replicas
			$sql = "
				SELECT n.NSID, n.NAMESPACE, l.LOBID, l.VARIABLE, l.SPACE, l.CHECKSUM, l.REPLICAS, COUNT(1) ACTIVE
				FROM synd_storage_namespace n, synd_storage_lob l, synd_storage_replica r, synd_storage_device d
				WHERE 
					n.nsid = l.nsid AND
					l.lobid = r.lobid AND
					l.checksum = r.checksum AND
					l.deleted = 0 AND
					d.devid = r.devid AND
					d.failed < ".(time()-$this->_retryInterval)."
				GROUP BY n.nsid, n.namespace, l.lobid, l.variable, l.space, l.checksum, l.replicas
				HAVING COUNT(1) < l.replicas";
		
			$lobs = $this->_db->getAll($sql, $offset, 1000);
			if (SyndLib::isError($lobs)) {
				trigger_error($lobs->getMessage(), E_USER_WARNING);
				return;
			}

			if (empty($lobs))
				break;

			$devices = array();
			shuffle($lobs);
			
			foreach ($lobs as $lob) {
				if (false != ($uri = $this->get($lob['NAMESPACE'], $lob['VARIABLE']))) {
					$sql = "
						SELECT d.devid, d.urn 
						FROM synd_storage_device d
							LEFT JOIN (
								SELECT r.devid FROM synd_storage_lob l, synd_storage_replica r
								WHERE 
									l.lobid = r.lobid AND 
									l.checksum = r.checksum AND 
									l.lobid = ".$this->_db->quote($lob['LOBID']).") d2
							ON (d2.devid = d.devid)
						WHERE 
							d.nsid = ".$this->_db->quote($lob['NSID'])." AND
							d.failed < ".(time()-$this->_retryInterval)." AND
							d2.devid IS NULL
						ORDER BY (d.space_total - d.space_used) * (1 - ".$this->_db->rand()." / 5) DESC";

					$urns = $this->_db->getAssoc($sql);
					if (SyndLib::isError($urns)) {
						trigger_error($urns->getMessage(), E_USER_WARNING);
						return;
					}

					foreach ($urns as $devid => $urn) {
						if (!isset($devices[$devid])) {
							if (null === ($transport = Activator::getInstance($urn, true, true))) {
								$this->_failDevice($devid);
								continue;
							}
							$devices[$devid] = $transport;
						}

						if (!$devices[$devid]->replicate(
							$lob['NAMESPACE'], $lob['VARIABLE'], $uri, $lob['CHECKSUM'], 
							$lob['SPACE'], array($this, '_callback_replica'), $devid))
							$this->_failDevice($devid);
						else if (++$lob['ACTIVE'] >= $lob['REPLICAS'])
							break;
					}
				}
			}

			$this->_devStatistics();
		}
	}
	
	/**
	 * Replication success callback
	 *
	 * This method is called by devices after they have successfully
	 * replicated a LOB.
	 *
	 * @access	protected
	 */
	function _callback_replica($device, $namespace, $variable, $checksum, $devid) {
		$this->_db->begin();

		// Lock single LOB row (select nsid in subquery to prevent locking the namespace)
		$sql = "
			SELECT l.LOBID, l.CHECKSUM FROM synd_storage_lob l
			WHERE 
				l.nsid = (
					SELECT n.nsid FROM synd_storage_namespace n
					WHERE n.namespace = ".$this->_db->quote($namespace).") AND
				l.variable = ".$this->_db->quote($variable)." AND
				l.deleted = 0
			FOR UPDATE";
		$lobs = $this->_db->getAll($sql);
		
		if (SyndLib::isError($lobs)) {
			$device->delete($namespace, $variable);
			$this->_db->rollback();
			trigger_error($lob->getMessage(), E_USER_WARNING);
		}
		else if (false === ($lob = reset($lobs)) || 
			$checksum != $lob['CHECKSUM'] || !$this->_addReplica($devid, $lob['LOBID'], $checksum)) {
			$device->delete($namespace, $variable);
			$this->_db->rollback();
		}
		else {
			$this->_db->commit();
		}
	}

	/**
	 * Removes redundant replicas
	 * @access	protected
	 */
	function _callback_normalize() {
		for ($ts=time();;) {
			$sql = "
				SELECT l.lobid 
				FROM synd_storage_lob l, synd_storage_replica r, synd_storage_device d
				WHERE 
					l.lobid = r.lobid AND
					l.checksum = r.checksum AND
					d.devid = r.devid AND
					d.failed < ".$this->_db->quote($ts)."
				GROUP BY l.lobid, l.replicas
				HAVING COUNT(1) > l.replicas";

			$lobids = $this->_db->getCol($sql, 0, 0, 1000);
			if (SyndLib::isError($lobids)) {
				trigger_error($lobids->getMessage(), E_USER_WARNING);
				return;
			}

			if (empty($lobids))
				break;
			
			shuffle($lobids);
			foreach ($lobids as $lobid) {
				$this->_db->begin();
				
				// Lock LOB and replicas (not namespace or device)
				$sql = "
					SELECT l.VARIABLE, l.REPLICAS, l.SPACE, r.DEVID, 
						(SELECT n.namespace FROM synd_storage_namespace n
						 WHERE n.nsid = l.nsid) NAMESPACE,
						(SELECT d.urn FROM synd_storage_device d
						 WHERE d.devid = r.devid AND d.failed < ".(time()-$this->_retryInterval).") URN
					FROM synd_storage_lob l, synd_storage_replica r
					WHERE
						l.lobid = r.lobid AND
						l.checksum = r.checksum AND
						l.lobid = ".$this->_db->quote($lobid)."
					FOR UPDATE";

				$replicas = $this->_db->getAll($sql);
				if (SyndLib::isError($replicas)) {
					$this->_db->rollback();
					trigger_error($replicas->getMessage(), E_USER_WARNING);
					return;
				}
				
				// Validate replicas on devices
				foreach ($replicas as $key => $replica) {
					if (empty($replica['URN']) || null === ($device = Activator::getInstance($replica['URN'], true, true)))
						unset($replicas[$key]);
					else if (!$device->get($replica['NAMESPACE'], $replica['VARIABLE'], $replica['SPACE'])) {
						$this->_failReplica($replica['DEVID']);
						unset($replicas[$key]);
					}
				}

				if (empty($replicas)) {
					$this->_db->rollback();
					continue;
				}
				
				// Remove replicas from storage device
				$r = reset($replicas);
				$d = count($replicas) - $r['REPLICAS'];
				
				foreach ($replicas as $replica) {
					if (--$d < 0)
						break;
					if (null === ($device = Activator::getInstance($replica['URN'], true, true)) ||
						!$device->delete($replica['NAMESPACE'], $replica['VARIABLE'])) {
						$this->_failDevice($replica['DEVID']);
						break;
					}

					// Delete replica entry
					$sql = "
						DELETE FROM synd_storage_replica
						WHERE
							devid = ".$this->_db->quote($replica['DEVID'])." AND
							lobid = ".$this->_db->quote($lobid);

					$result = $this->_db->query($sql);
					if (SyndLib::isError($result)) {
						$this->_db->commit();
						trigger_error($result->getMessage(), E_USER_WARNING);
						return;
					}
				}

				$this->_db->commit();
			}

			$this->_devStatistics();
		}
	}
	
	/**
	 * Removes deleted LOB from storage devices
	 * @access	protected
	 */
	function _callback_garbage_collect_lobs() {
		for ($ts=time();;) {
			$sql = "
				SELECT DISTINCT l.lobid 
				FROM synd_storage_lob l, synd_storage_replica r, synd_storage_device d
				WHERE 
					l.deleted = 1 AND
					l.lobid = r.lobid AND
					d.devid = r.devid AND
					d.failed < ".$this->_db->quote($ts);

			$lobids = $this->_db->getCol($sql, 0, 0, 1000);
			if (SyndLib::isError($lobids)) {
				trigger_error($lobids->getMessage(), E_USER_WARNING);
				return;
			}

			if (empty($lobids))
				break;
			
			shuffle($lobids);
			foreach ($lobids as $lobid) {
				$this->_db->begin();
				
				// Lock LOB and replicas (not namespace or device)
				$sql = "
					SELECT l.VARIABLE, r.DEVID, 
						(SELECT n.namespace FROM synd_storage_namespace n
						 WHERE n.nsid = l.nsid) NAMESPACE,
						(SELECT d.urn FROM synd_storage_device d
						 WHERE d.devid = r.devid AND d.failed < ".(time()-$this->_retryInterval).") URN
					FROM synd_storage_lob l, synd_storage_replica r
					WHERE
						l.deleted = 1 AND
						l.lobid = r.lobid AND
						l.lobid = ".$this->_db->quote($lobid)."
					FOR UPDATE";

				$replicas = $this->_db->getAll($sql);
				if (SyndLib::isError($replicas)) {
					$this->_db->rollback();
					trigger_error($replicas->getMessage(), E_USER_WARNING);
					return;
				}
				
				if (empty($replicas)) {
					$this->_db->rollback();
					continue;
				}
				
				// Remove replicas from storage device
				foreach ($replicas as $replica) {
					if (empty($replica['URN']) || null === ($device = Activator::getInstance($replica['URN'], true, true)))
						continue;

					// Delete replica from storage device
					if (!$device->delete($replica['NAMESPACE'], $replica['VARIABLE'])) {
						$this->_failDevice($replica['DEVID']);
						continue;
					}

					// Delete replica entry
					$sql = "
						DELETE FROM synd_storage_replica
						WHERE
							devid = ".$this->_db->quote($replica['DEVID'])." AND
							lobid = ".$this->_db->quote($lobid);

					$result = $this->_db->query($sql);
					if (SyndLib::isError($result)) {
						$this->_db->commit();
						trigger_error($result->getMessage(), E_USER_WARNING);
						return;
					}
				}

				// Delete LOB entry
				$sql = "
					DELETE FROM synd_storage_lob
					WHERE 
						lobid = ".$this->_db->quote($lobid)." AND
						NOT EXISTS (
							SELECT 1 FROM synd_storage_replica r 
							WHERE r.lobid = synd_storage_lob.lobid)";

				$result = $this->_db->query($sql);
				if (SyndLib::isError($result)) {
					$this->_db->commit();
					trigger_error($result->getMessage(), E_USER_WARNING);
					return;
				}
				
				$this->_db->commit();
			}

			$this->_devStatistics();
		}
	}
	
	/**
	 * Removes stale replicas from storage devices
	 * @access	protected
	 */
	function _callback_garbage_collect_replicas() {
		for ($ts=time();;) {
			$sql = "
				SELECT n.NAMESPACE, l.LOBID, l.VARIABLE, r.DEVID
				FROM synd_storage_namespace n, synd_storage_lob l, synd_storage_replica r, synd_storage_device d
				WHERE 
					n.nsid = l.nsid AND
					l.lobid = r.lobid AND
					l.checksum != r.checksum AND
					d.devid = r.devid AND
					d.failed < ".$this->_db->quote($ts);

			$replicas = $this->_db->getAll($sql, 0, 1000);
			if (SyndLib::isError($replicas)) {
				trigger_error($replicas->getMessage(), E_USER_WARNING);
				return;
			}

			if (empty($replicas))
				break;
			
			shuffle($replicas);
			foreach ($replicas as $replica) {
				$this->_db->begin();
				
				// Lock replica (not LOB or device)
				$sql = "
					SELECT r.DEVID, 
						(SELECT d.urn FROM synd_storage_device d
						 WHERE d.devid = r.devid AND d.failed < ".(time()-$this->_retryInterval).") URN
					FROM synd_storage_replica r
					WHERE
						r.checksum != 
							(SELECT l.checksum FROM synd_storage_lob l
							 WHERE l.lobid = r.lobid) AND
						r.lobid = ".$this->_db->quote($replica['LOBID'])." AND
						r.devid = ".$this->_db->quote($replica['DEVID'])."
					FOR UPDATE";

				$repinfo = $this->_db->getAll($sql);
				if (SyndLib::isError($repinfo)) {
					$this->_db->rollback();
					trigger_error($repinfo->getMessage(), E_USER_WARNING);
					return;
				}
				
				if (empty($repinfo)) {
					$this->_db->rollback();
					continue;
				}
				
				// Remove replica from storage device
				if (null === ($device = Activator::getInstance($repinfo[0]['URN'], true, true)) ||
					!$device->delete($replica['NAMESPACE'], $replica['VARIABLE'])) {
					$this->_db->rollback();
					continue;
				}
				
				// Delete replica entry
				$sql = "
					DELETE FROM synd_storage_replica
					WHERE
						devid = ".$this->_db->quote($replica['DEVID'])." AND
						lobid = ".$this->_db->quote($replica['LOBID']);

				$result = $this->_db->query($sql);
				if (SyndLib::isError($result)) {
					$this->_db->rollback();
					trigger_error($result->getMessage(), E_USER_WARNING);
					return;
				}

				$this->_db->commit();
			}
		}

		$this->_devStatistics();
	}

	/**
	 * Device repair task
	 *
	 * Will ask the failed device to checksum its stored replicas
	 * in order to discover missing or corrupted LOBs.
	 *
	 * The process is run inside a transaction to prevent inserted or 
	 * removed from affecting the result. This requires REPEATABLE-READ 
	 * isolation level (default in InnoDB and Oracle)
	 *
	 * Using SERIALIZABLE transaction level is not recommended as 
	 * it would lock its LOBs for the duration of the repair process.
	 *
	 * The process keeps a write lock on the device in order to prevent
	 * multiple trackers from all resyncing the device.
	 *
	 * @access	protected
	 * @param	integer	Device id to repair
	 * @param	bool	Bring device offline during repair
	 */
	function _callback_repair($devid, $offline = true) {
		if ($offline) {
			$sql = "
				UPDATE synd_storage_device
				SET failed = NULL
				WHERE 
					devid = ".$this->_db->quote($devid)." AND
					failed IS NOT NULL";

			// Bring device offline during repair
			if (SyndLib::isError($result = $this->_db->query($sql))) {
				trigger_error($result->getMessage(), E_USER_WARNING);
				return;
			}

			// Some other tracker might already be running repairs
			if (!$this->_db->affectedRows())
				return;
		}
		
		$this->_db->begin();
		
		$sql = "
			SELECT d.urn FROM synd_storage_device d
			WHERE d.devid = ".$this->_db->quote($devid);

		if (null == ($urn = $this->_db->getOne($sql)) || SyndLib::isError($urn) ||
			null === ($device = Activator::getInstance($urn, true, true))) {
			$this->_db->rollback();
			return;
		}
		
		for ($offset=0; ; $offset+=50) {
			$sql = "
				SELECT n.NAMESPACE, l.LOBID, l.VARIABLE, l.CHECKSUM, l.SPACE
				FROM synd_storage_namespace n, synd_storage_lob l, synd_storage_replica r
				WHERE
					l.deleted = 0 AND
					l.nsid = n.nsid AND
					l.lobid = r.lobid AND
					(l.checksum = r.checksum OR r.checksum = '-') AND
					r.devid = ".$this->_db->quote($devid);
		
			$rows = $this->_db->getAll($sql, $offset, 50);
			if (SyndLib::isError($rows)) {
				$this->_db->rollback();
				trigger_error($rows->getMessage(), E_USER_WARNING);
				return;
			}
			
			if (empty($rows))
				break;
			
			// Build checksum requests
			$lobs = array();
			$namespaces = array();
			foreach ($rows as $lob) {
				$lobs[$lob['NAMESPACE']][$lob['LOBID']] = $lob;
				$namespaces[$lob['NAMESPACE']][] = $lob['VARIABLE'];
			}
			
			// Read checksums from device
			foreach ($namespaces as $namespace => $variables) {
				if (false === ($actual = $device->checksum($namespace, $variables))) {
					$this->_failDevice($devid);
					$this->_db->commit();
					return;
				}
				
				// Resync missing or currupt LOBs
				foreach ($lobs[$namespace] as $lobid => $lob) {
					if (!isset($actual[$lob['VARIABLE']]) || $actual[$lob['VARIABLE']] != $lob['CHECKSUM']) {
						$sql = "
							SELECT d.urn 
							FROM synd_storage_lob l, synd_storage_replica r, synd_storage_device d
							WHERE 
								l.lobid = r.lobid AND 
								l.checksum = r.checksum AND
								d.devid = r.devid AND
								d.devid != ".$this->_db->quote($devid)." AND
								d.failed < ".(time()-$this->_retryInterval)." AND
								l.lobid = ".$this->_db->quote($lob['LOBID']);
						$sources = (array)$this->_db->getCol($sql);
						
						foreach ($sources as $srcurn) {
							if (null !== ($source = Activator::getInstance($srcurn, true, true)) &&
								null != ($uri = $source->get($lob['NAMESPACE'], $lob['VARIABLE'], $lob['SPACE']))) {
								// Copy LOB syncronously
								if ($device->set($lob['NAMESPACE'], $lob['VARIABLE'], $uri, $lob['CHECKSUM'], $lob['SPACE']))
									continue 2;
								// Check if target device failed
								else if (null != ($uri = $devices[$srcdevid]->get($lob['NAMESPACE'], $lob['VARIABLE'], $lob['SPACE']))) {
									$this->_failDevice($devid);
									$this->_db->commit();
									return;
								}
							}
						}
						
						// Remove replica if repair failed
						$sql = "
							UPDATE synd_storage_replica
							SET checksum = '-'
							WHERE 
								devid = ".$this->_db->quote($devid)." AND
								lobid = ".$this->_db->quote($lob['LOBID']);
						
						if (SyndLib::isError($result = $this->_db->query($sql))) {
							$this->_db->commit();
							trigger_error($result->getMessage(), E_USER_WARNING);
							return;
						}
					}
				}
			}
		}
		
		// Bring device back online
		$sql = "
			UPDATE synd_storage_device
			SET failed = 0
			WHERE devid = ".$this->_db->quote($devid);

		if (SyndLib::isError($result = $this->_db->query($sql))) {
			$this->_db->rollback();
			trigger_error($result->getMessage(), E_USER_WARNING);
			return;
		}
		
		$this->_db->commit();
	}
	
	/**
	 * Runs the replication and garbage collection routines
	 * @access	protected
	 */
	function _hook_cron(&$result) {
		// Check for devices in need of repair
		$sql = "
			SELECT d.devid FROM synd_storage_device d
			WHERE d.failed IS NULL";
		foreach ($this->_db->getCol($sql) as $devid) 
			$this->_task("repair.$devid", array($this, '_callback_repair'), array($devid, false));

		$this->_task('replicate', array($this, '_callback_replicate'));
		$this->_task('normalize', array($this, '_callback_normalize'));
		$this->_task('garbage_collect_lobs', array($this, '_callback_garbage_collect_lobs'));
		$this->_task('garbage_collect_replicas', array($this, '_callback_garbage_collect_replicas'));
	}

	function _nsid($namespace) {
		$sql = "
			SELECT n.nsid FROM synd_storage_namespace n
			WHERE n.namespace = ".$this->_db->quote($namespace);
		
		if (null == ($nsid = $this->_db->getOne($sql))) {
			if (empty($this->_config['devices'])) {
				trigger_error('No storage devices available in $synd_config[\'tracker\'][\'devices\']', E_USER_WARNING);
				return null;
			}
			
			$this->_db->begin();
			$result = $this->_db->insert('synd_storage_namespace', array(
				'NAMESPACE' => $namespace));

			if (SyndLib::isError($result) || !isset($result['NSID'])) {
				$this->_db->rollback();
				return null;
			}

			$nsid = $result['NSID'];
			
			// Add default storage devices
			foreach ($this->_config['devices'] as $urn) {
				if (null !== ($device = Activator::getInstance($urn, true, true)) && false !== ($size = $device->getFreeSpace())) {
					$result = $this->_db->insert('synd_storage_device', array(
						'NSID'			=> $nsid,
						'URN'			=> $urn,
						'SPACE_TOTAL'	=> round($size * 0.75)));
					if (SyndLib::isError($result)) {
						$this->_db->rollback();
						trigger_error($result->getMessage(), E_USER_WARNING);
						return null;
					}
				}
				else {
					$this->_db->rollback();
					return null;
				}
			}
			
			$this->_db->commit();
		}
		
		return $nsid;
	}
	
	/**
	 * Update the main LOB info
	 * @access	private
	 * @return	integer		Returns the LOBID, null on failure
	 */
	function _add($nsid, $variable, $checksum, $size, $replicas) {
		$data = array(
			'NSID'		=> $nsid,
			'VARIABLE'	=> $variable,
			'REPLICAS'	=> $replicas,
			'CHECKSUM'	=> $checksum,
			'SPACE'		=> $size,
			'MODIFIED'	=> time(),
			'DELETED'	=> 0);
		$result = $this->_db->update('synd_storage_lob', $data);
		
		if (SyndLib::isError($result)) {
			trigger_error($result->getMessage(), E_USER_WARNING);
			return false;
		}
		
		if (!$this->_db->affectedRows()) {
			$data['CREATED'] = $data['MODIFIED'];
			$result = $this->_db->insert('synd_storage_lob', $data);
		
			if (SyndLib::isError($result)) {
				trigger_error($result->getMessage(), E_USER_WARNING);
				return false;
			}
		}
		
		if (isset($result['LOBID']))
			$lobid = $result['LOBID'];
		else {
			$sql = "
				SELECT l.lobid FROM synd_storage_lob l
				WHERE 
					l.nsid = ".$this->_db->quote($nsid)." AND
					l.variable = ".$this->_db->quote($variable);
			$lobid = $this->_db->getOne($sql);
		}
		
		return $lobid;
	}

	/**
	 * Update replicate info
	 * @access	private
	 * @return	bool	Returns true on success, false on failure
	 */
	function _addReplica($devid, $lobid, $checksum) {
		$result = $this->_db->replace('synd_storage_replica', array(
			'DEVID'		=> $devid,
			'LOBID'		=> $lobid,
			'CHECKSUM'	=> $checksum));

		if (SyndLib::isError($result)) {
			trigger_error($result->getMessage(), E_USER_WARNING);
			return false;
		}

		return true;
	}

	/**
	 * Updates device statistics (used space, ..)
	 * @access	private
	 */
	function _devStatistics() {
		$sql = "
			UPDATE synd_storage_device
			SET space_used = (
				SELECT SUM(l.space) FROM synd_storage_lob l, synd_storage_replica r
				WHERE l.lobid = r.lobid AND r.devid = synd_storage_device.devid)";
		$this->_db->query($sql);
	}
	
	/**
	 * Marks a device failed
	 *
	 * This is most likely because of network or other temporary 
	 * infrastructure failures, the device is not resynced.
	 *
	 * @access	private
	 */
	function _failDevice($devid) {
		$sql = "
			UPDATE synd_storage_device
			SET failed = ".$this->_db->quote(time())."
			WHERE 
				devid = ".$this->_db->quote($devid)." AND
				failed IS NOT NULL";
		$result = $this->_db->query($sql);
		return !SyndLib::isError($result);
	}
	
	/**
	 * Indicate that a LOB replica is failed or corrupt
	 *
	 * This will bring the device offline and trigger a complete 
	 * resync in order to ensure data consistency.
	 *
	 * @access	private
	 */
	function _failReplica($devid) {
		if (!$this->_failDevice($devid))
			return false;
		$this->_task("repair.$devid", array($this, '_callback_repair'), array($devid));
		return true;
	}

	/**
	 * Returns the available storage devices given a LOB id
	 * @param	string	Namespace
	 * @param	string	LOB id
	 * @return	array	List of LobStorageDevice
	 */
	function getDevices($namespace, $variable) {
		$devices = array();
		foreach ($this->getReplicas($namespace, $variable) as $replica) {
			if (null !== ($device = Activator::getInstance($replica['URN'], true, true)))
				$devices[$replica['DEVID']] = $device;
			else
				$this->_failDevice($replica['DEVID']);
		}
		return $devices;
	}
	
	/**
	 * Returns information on online replicas
	 * @access	protected
	 */
	function getReplicas($namespace, $variable) {
		$sql = "
			SELECT d.DEVID, d.URN, l.SPACE
			FROM synd_storage_namespace n, synd_storage_lob l, synd_storage_replica r, synd_storage_device d
			WHERE 
				n.nsid = l.nsid AND
				l.lobid = r.lobid AND 
				l.checksum = r.checksum AND
				l.deleted = 0 AND
				d.devid = r.devid AND
				d.failed < ".(time()-$this->_retryInterval)." AND
				n.namespace = ".$this->_db->quote($namespace)." AND
				l.variable = ".$this->_db->quote($variable);

		$replicas = (array)$this->_db->getAll($sql);
		shuffle($replicas);
		return $replicas;
	}
	
	/**
	 * @access	private
	 */
	function _task($id, $callback, $args = null) {
		require_once 'core/lib/SyndTaskHandler.class.inc';
		$handler = SyndTaskHandler::factory('tracker');
		if (!$handler->isScheduled($id))
			$handler->appendTask(SyndType::factory('task', $id, $callback, $args), $id);
	}
	
	/**
	 * @access	private
	 */
	function _isScheduled($id) {
		require_once 'core/lib/SyndTaskHandler.class.inc';
		$handler = SyndTaskHandler::factory('tracker');
		return $handler->isScheduled($id);
	}

	function _callback_menu_admin(&$result, $menu) {
		if ($this->isPermitted('admin'))
			$menu->append(tpl_link('system','tracker','admin'), SyndLib::translate('Storage'));
	}
	
	function _view_admin(Request $request, Template $page) {
		require_once 'core/lib/Template.class.inc';
		require_once 'core/lib/SyndTaskHandler.class.inc';
		if (!$this->isPermitted('admin'))
			throw new ForbiddenException();
		
		$sql = "SELECT d.DEVID, d.URN, d.FAILED FROM synd_storage_device d";
		$stats = array();
		$failed = array();
		$rows = $this->_db->getAll($sql);
		
		foreach ($rows as $row) {
			try {
				if (!is_numeric($row['FAILED']) || $row['FAILED'] >= time()-$this->_retryInterval)
					$failed[] = $row['URN'];
				else if (null === ($device = Activator::getInstance($row['URN'], true, true)) ||
					false === ($stats[$row['URN']]['total'] = $device->getTotalSpace()) ||
					false === ($stats[$row['URN']]['free'] = $device->getFreeSpace())) {
					trigger_error("Failed to fetch stats from device '{$row['URN']}'", E_USER_WARNING);
					//$this->_failDevice($row['DEVID']);
				}
			}
			catch (Exception $e) {
				//$this->_failDevice($row['DEVID']);
				trigger_error($e->getMessage(), E_USER_WARNING);
			}
		}
		
		$sql = "
			SELECT n.NSID, n.NAMESPACE, 
				l1.cnt LOBCOUNT, l1.space LOBSPACE,
				l2.cnt DELCOUNT, l2.space DELSPACE,
				l3.cnt REPCOUNT
			FROM synd_storage_namespace n
				LEFT JOIN (
					SELECT l.nsid, COUNT(1) cnt, SUM(l.space) space FROM synd_storage_lob l
					WHERE l.deleted = 0
					GROUP BY l.nsid) l1
				ON (l1.nsid = n.nsid)
				LEFT JOIN (
					SELECT l.nsid, COUNT(1) cnt, SUM(l.space) space FROM synd_storage_lob l
					WHERE l.deleted = 1
					GROUP BY l.nsid) l2
				ON (l2.nsid = n.nsid)
				LEFT JOIN (
					SELECT t1.nsid, COUNT(1) cnt FROM (
						SELECT l.nsid, l.lobid FROM synd_storage_lob l
							LEFT JOIN synd_storage_replica  r
							ON (r.lobid = l.lobid AND l.checksum = r.checksum)
						WHERE 
							l.deleted = 0
						GROUP BY l.nsid, l.lobid, l.replicas
						HAVING COUNT(r.lobid) < l.replicas) t1
					GROUP BY t1.nsid) l3
				ON (l3.nsid = n.nsid)
			GROUP BY n.nsid, n.namespace, l1.cnt, l2.cnt, l3.cnt, l1.space, l2.space
			ORDER BY n.namespace";
		$namespaces = $this->_db->getAll($sql);

		$sql = "
			SELECT d.NSID, d.DEVID, d.URN, d.FAILED, d.space_total SPACE, 
				COUNT(l1.lobid) LOBCOUNT, SUM(l1.space) LOBSPACE,
				COUNT(l2.lobid) DELCOUNT, SUM(l2.space) DELSPACE
			FROM synd_storage_device d
				LEFT JOIN synd_storage_replica r
				ON (d.devid = r.devid)
				LEFT JOIN synd_storage_lob l1
				ON (r.lobid = l1.lobid AND l1.deleted = 0)
				LEFT JOIN synd_storage_lob l2
				ON (r.lobid = l2.lobid AND l2.deleted = 1)
			GROUP BY d.nsid, d.devid, d.urn, d.failed, d.space_total
			ORDER BY d.nsid, d.urn";
		$devices = $this->_db->getAll($sql);
		
		$page->assign('stats', $stats);
		$page->assign('failed', $failed);
		$page->assign('devices', $devices);
		$page->assign('namespaces', $namespaces);
		$page->assign('module', $this);

		$handler = SyndTaskHandler::factory('tracker');
		$page->assign('replScheduled', $handler->isScheduled('replicate'));
		
		return array('content' => $page->fetch(tpl_design_path('module/tracker/admin.tpl')));
	}

	function _view_saveDevices(Request $request, Template $page) {
		if (!$this->isPermitted('admin'))
			throw new ForbiddenException();
		
		if (!empty($request['devices'])) {
			foreach ((array)$request['devices'] as $devid => $size) {
				if (is_numeric($size) && $size >= 0) {
					$result = $this->_db->update('synd_storage_device', array(
						'DEVID'			=> $devid,
						'SPACE_TOTAL'	=> $size*1024*1024));
				}
			}
		}

		if (!empty($request['device'])) {
			foreach ((array)$request['device'] as $nsid => $devinf) {
				if ('' != $devinf['URN']) {
					if (is_numeric($devinf['SPACE']))
						$size = $devinf['SPACE']*1024*1024;
					else if (null === ($device = Activator::getInstance($devinf['URN'], true, true)) || false === ($size = $device->getFreeSpace()))
						continue;
					
					$result = $this->_db->insert('synd_storage_device', array(
						'NSID'			=> $nsid,
						'URN'			=> $devinf['URN'],
						'SPACE_TOTAL'	=> $size));
					$this->_task('replicate', array($this, '_callback_replicate'));
				}
			}
		}
	}
	
	function _view_replicate(Request $request, Template $page) {
		if (!$this->isPermitted('admin'))
			throw new ForbiddenException();
		$this->_task('replicate', array($this, '_callback_replicate'));
	}
	
	function _view_repair(Request $request, Template $page) {
		if (!$this->isPermitted('admin'))
			throw new ForbiddenException();
		if (!empty($request[0]))
			$this->_task("repair.{$request[0]}", array($this, '_callback_repair'), array($request[0], false));
	}
}
