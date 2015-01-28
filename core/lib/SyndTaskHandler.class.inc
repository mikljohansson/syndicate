<?php
require_once 'core/lib/SyndLib.class.inc';

if (!defined('SIGKILL'))
	define('SIGKILL', 9);
if (!defined('SIGTERM'))
	define('SIGTERM', 15);

/**
 * Offline task interface
 * @access		public
 * @package		synd.core.model
 */
class SyndTask {
	var $_config = null;
	
	function SyndTask() {
		global $synd_config;
		$this->_config = $synd_config;
	}
	
	/**
	 * The worker callback function
	 * @param	SyndTaskHandler	The parent task handler
	 * @param	string					Path to task file being executed
	 */
	function run($handler) {
		global $synd_config;
		$synd_config = $this->_config;
	}
}

/**
 * Task process handler.
 *
 * Runs offline tasks in a separate process, uses file locks for
 * syncronization. Tasks are serialized and run one by one in their
 * own process so any exceptions or hangs doesn't affect other tasks.
 *
 * Output is piped to a log file with a maximum size and rate to
 * avoid filling up the disk in case of runaway processes.
 *
 * Note: Oracle (OCI8) connections aren't able to survive fork()'s,
 * giving 'end-of-file on communication channel' errors, so we launch 
 * an independent process instead.
 *
 * @access		public
 * @package		synd.core.model
 */
class SyndTaskHandler extends SyndTask {
	var $_id = null;
	
	var $_lockStack = array();
	var $_lockHandles = array();
	var $_failureCount = 0;
	
	function SyndTaskHandler($id) {
		parent::SyndTask();
		$this->_id = $id;
	}
	
	/**
	 * Factory method for task handlers
	 * @param	string	Id of taskhandler
	 * @return	SyndTaskHandler
	 */
	static function factory($id) {
		global $synd_config;
		static $taskHandlers = array();
		if (!isset($taskHandlers[$id]))
			$taskHandlers[$id] = new SyndTaskHandler($id);
		return $taskHandlers[$id];
	}

	function __sleep() {
		$vars = get_object_vars($this);
		unset($vars['_lockStack']);
		unset($vars['_lockHandles']);
		return array_keys($vars);
	}
	
	function setFailureCount($failureCount) {
		$this->_failureCount = $failureCount;
	}

	function getFailureCount() {
		return $this->_failureCount;
	}

	function appendTask($task, $id = null, $timelimit = 0) {
		if (!$this->lockHandler())
			return trigger_error('Could not aquire lock on handler.', E_USER_WARNING);
		
		$storage = $this->getStoragePath().'tasks/';
		if (!SyndLib::createDirectory($storage))
			return trigger_error('Could not create work directory.', E_USER_WARNING);
		
		if (null == $id)
			$id = get_class($task).'.'.md5(uniqid(''));
		
		$buffer = serialize(array('task' => serialize($task), 'timelimit' => $timelimit));
		if (!SyndLib::file_put_contents("{$storage}{$id}.obj", $buffer))
			return trigger_error('Could not write task file.', E_USER_WARNING);

		$this->runHandler();
		$this->unlockHandler();
	}

	/**
	 * Checks if a task is scheduled
	 * @param	string	Id of the task to check for
	 */
	function isScheduled($id) {
		if (file_exists($this->getStoragePath()."tasks/{$id}.obj")) {
			$this->runHandler();
			return true;
		}
		return false;
	}
	
	/**
	 * Checks if the handler process is running (non blocking)
	 * @return	bool
	 */
	function isRunning() {
		$pidFile = $this->getStoragePath().'handler.pid';
		if (file_exists($pidFile) && is_numeric($pid = file_get_contents($pidFile))) {
			// Check if process is alive
			if (0 != $pid)
				return posix_kill($pid, 0);
			
			// Temporary pid, check the mtime to allow the handler a few seconds to start
			$mtime = filemtime($pidFile);
			return $mtime >= time() - 30;
		}
		
		return false;
	}
	
	/**
	 * Starts the handler process
	 * @access	protected
	 */
	function runHandler() {
		global $synd_config;
		if (!$this->lockHandler())
			return trigger_error('Could not aquire lock on handler.', E_USER_WARNING);
		
		// Check if handler is running while holding lock, to avoid race condition
		if ($this->isRunning()) {
			$this->unlockHandler();
			return;
		}
		
		if (!SyndLib::createDirectory($this->getStoragePath())) {
			$this->unlockHandler();
			return trigger_error('Could not create storage directory.', E_USER_WARNING);
		}

		// Write handler task file
		$handler = $this->getStoragePath().'handler.obj';
		$buffer = serialize(array('task' => serialize($this), 'timelimit' => null));
		if (!SyndLib::file_put_contents($handler, $buffer)) {
			$this->unlockHandler();
			return trigger_error("Could not write to '$handler'.", E_USER_WARNING);
		}

		// Write a temporary pid file to signal isRunning() that the handler is about to statr
		$pidFile = $this->getStoragePath().'handler.pid';
		if (!SyndLib::file_put_contents($this->getStoragePath().'handler.pid', 0)) {
			$this->unlockHandler();
			return trigger_error("Could not write temporary pid.", E_USER_WARNING);
		}
		
		// Start handler process 
		if (!is_resource($pd = $this->openProcess($handler, 0, $this->getOutputFile()))) {
			$this->unlockHandler();
			return trigger_error("Could not launch handler process.", E_USER_WARNING);
		}
		
		pclose($pd);
		$this->unlockHandler();
	}

	/**
	 * Handler process function
	 * @access	protected
	 */
	function run($handler) {
		parent::run($handler);
		ob_implicit_flush(1);
		
		$pidFile = $this->getStoragePath().'handler.pid';
		if (!SyndLib::file_put_contents($this->getStoragePath().'handler.pid', posix_getpid())) {
			$this->log("Handler: Failed to write pid file ($pidFile)\n");
			return 1;
		}
		
		$count = 0;
		$file = null;
		$mtime = null;

		for (;;) {
			// Find list of scheduled tasks
			if (!$this->lockHandler()) {
				$this->log('Could not aquire read lock on handler.');
				return 1;
			}
			
			$tasks = SyndLib::scanDirectory($this->getStoragePath().'tasks/');
			sort($tasks);
			
			if (null == ($file = reset($tasks))) {
				$this->log("Handler: Done $count tasks\n");
				
				// Must keep lock until task directory is deleted
				SyndLib::unlink($this->getStoragePath(), true);
				$this->unlockHandler();
				return;
			}

			// Unserialize task to get the timelimit field
			if (null == ($buffer = file_get_contents($file)) || null == ($mtime = filemtime($file)) || null == ($info = unserialize($buffer))) {
				unlink($file);
				$this->log("Handler: Could not read taskfile ($file)\n");
			}
			
			// Release lock while processing task
			$this->unlockHandler();

			// Attempt to run task and retry on failure
			$this->log("Handler: Launching worker process ($file)\n");
			$failures = 0;
			$status = 0;

			for (;;) {
				$pd = $this->openProcess($file, $info['timelimit']);
				$this->pipeOutput($pd);
				$status = pclose($pd);

				// Check if task succeeded or has failed too much
				if ($status == 0 || $failures >= 16) {
					if (!$this->lockHandler()) {
						$this->log('Could not aquire read lock on handler.');
						return 1;
					}

					// Delete task file if exit status is ok and file hasn't been changed
					clearstatcache();
					if ((null == $mtime || filemtime($file) == $mtime) && !unlink($file)) {
						$this->log("Handler: Could not delete taskfile ($file)\n");
					}

					$this->unlockHandler();
					break;
				}

				// Retry task in case of error
				$failures++;

				// Sleep a while to avoid looping too fast
				sleep($failures);
			}

			$count++;
			$this->log("Handler: Worker done (exit status $status after $failures failures)\n");
		}
	}

	/**
	 * Pipe output to logfile 
	 * @access	private
	 * @param	resource	Process descriptor
	 */
	function pipeOutput($pd) {
		static $length = 0;

		while (!feof($pd)) {
			$buffer = fgets($pd, 4096);

			// Max 100mb
			if ($length < 100000000) {
				$length += strlen($buffer);
				$this->log($buffer);
			}
		}
	}
	
	function log($message) {
		print $message;
		@ob_flush();
		flush();
	}
	
	/**
	 * Runs a task in a new process using popen()
	 * @access	private
	 * @return	resource
	 */
	function openProcess($file, $timelimit = 0, $output = null, $failures = 0) {
		if (null == ($interpretor = SyndLib::which('php'))) {
			trigger_error('PHP CLI does not exist or is not executable', E_USER_ERROR);
			return false;
		}
		$launcher = $this->getLauncher();
		$path = escapeshellarg(get_include_path());
		$id = escapeshellarg($this->_id);
		
		// Launch worker
		$command  = "$interpretor $launcher $file $path $timelimit $id $failures";
		$command .= null != $output ? " 1>$output 2>&1 &" : " 2>&1";
		return popen($command, 'r');
	}
	
	/**
	 * Aquire lock on the handler
	 * @access	private
	 */
	function lockHandler($id = 'handler', $mode = LOCK_EX) {
		if (empty($this->_lockStack[$id]) && !flock($this->getLockHandle($id), $mode))
			return false;
		if (!isset($this->_lockStack[$id]))
			$this->_lockStack[$id] = 0;
		$this->_lockStack[$id]++;
		return true;
	}
	
	/**
	 * Release lock on the handler
	 * @access	private
	 */
	function unlockHandler($id = 'handler') {
		assert('!empty($this->_lockStack[$id])');
		$this->_lockStack[$id]--;
		if (0 == $this->_lockStack[$id]) {
			flock($this->getLockHandle($id), LOCK_UN);
			fclose($this->getLockHandle($id));
			unset($this->_lockHandles[$id]);
		}
	}

	/**	
	 * @access	private
	 */
	function getLauncher() {
		return dirname(dirname(__FILE__)).'/script/launcher.inc';
	}
	
	/**	
	 * @access	private
	 */
	function getStoragePath() {
		global $synd_config;
		return $synd_config['dirs']['cache']."task/{$this->_id}/";
	}
	
	/**	
	 * @access	private
	 */
	function getOutputFile() {
		global $synd_config;
		return $synd_config['dirs']['cache']."task/{$this->_id}.log";
	}
	
	/**	
	 * @access	private
	 */
	function getLockHandle($id = 'handler') {
		global $synd_config;
		if (!isset($this->_lockHandles[$id])) {
			if (!SyndLib::createDirectory($dir = $this->getStoragePath()))
				return false;
			if (false === ($this->_lockHandles[$id] = @fopen($dir.$id.'.lock', 'w+')))
				return false;
		}
		return $this->_lockHandles[$id];
	}
}
