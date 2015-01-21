<?php
require_once 'core/Module.class.inc';
require_once 'core/model/ISyncable.class.inc';
require_once 'core/model/syncml/message.class.inc';
require_once 'core/model/syncml/node.class.inc';

/**
 * SyncML module
 *
 * Provides syncronization methods for mobile devices such as smart 
 * phones and PDAs.
 *
 * @access		protected
 * @package		synd.core.module
 * @link		http://www.wikipedia.org
 */
class synd_module_syncml extends Module {
	function __construct() {
		SyndLib::attachHook('syncronize', array($this, '_callback_syncronize'));
	}
	
	function _view_index(Request $request, Template $page) {
		global $HTTP_RAW_POST_DATA;
		set_time_limit(120);
		
		$headers = array_change_key_case(getallheaders(), CASE_LOWER);
		$message = new synd_syncml_message($HTTP_RAW_POST_DATA, 
			isset($headers['content-type']) ? $headers['content-type'] : null);

		$response = $message->getResponse();
		$content = $response->toString();
		
		$this->_log($message->_decode($HTTP_RAW_POST_DATA));
		$this->_log($message->_decode($content));
		
		header('Content-Type: '.$response->getContentType());
		header('Content-Length: '.strlen($content));
		return array('content' => $content, 'page' => null);
	}
	
	function _log($message, $file = null) {
		if (!is_scalar($message)) {
			ob_start();
			print_r($message);
			$message = ob_get_clean();
		}
		
		$fp = null != $file ? fopen($file, 'w') : fopen("/tmp/syncml-{$_SERVER['SERVER_NAME']}.log", 'a');
		fwrite($fp, date('Y-m-d H:i:s')." - {$_SERVER['REMOTE_ADDR']} - ".strlen($message)." bytes - {$_SERVER['HTTP_ACCEPT']}\n");
		fwrite($fp, trim($message)."\n\n");
		fclose($fp);
	}
	
	function _callback_syncronize(&$result, $name, $user, $message) {
		if ('devinf' == substr(strtolower(trim($name,'./')),0,6)) {
			$result[] = new SyncMLDeviceInfoCollection($message[0]);
			return true;
		}
	}
}

/**
 * Implements the ./devinf10 and ./devinf11 collections
 */
class SyncMLDeviceInfoCollection extends ISyncableCollection {
	var $_message = null;
	
	function SyncMLDeviceInfoCollection($message) {
		$this->_message = $message;
	}
	
	function isNull() {
		return false;
	}

	function getName() {
		return 'devinf11';
	}
	
	function getContents() {
		$info = synd_syncml_node::factory('DevInf');
		$info->setAttribute('xmlns', 'syncml:devinf');
		$info->appendChild(new synd_syncml_node('VerDTD', '1.1'));
		$info->appendChild(new synd_syncml_node('Man', 'Synd (http://www.synd.info/)'));
		$info->appendChild(new synd_syncml_node('Mod', 'SyncML module'));
		$info->appendChild(new synd_syncml_node('OEM', 'Synd (http://www.synd.info/)'));
		$info->appendChild(new synd_syncml_node('SwV', '1.0'));
		$info->appendChild(new synd_syncml_node('DevID', tpl_request_host().tpl_view('syncml')));
		$info->appendChild(new synd_syncml_node('DevTyp', 'server'));

		$persistent = $this->_message->getPersistentSession();
		$devinfo = $persistent->getDeviceInfo();
		$capabilities = array();
		
		foreach (array_keys($sourceRefs = $devinfo->selectNodes('DataStore', 'SourceRef')) as $key) {
			if (null !== ($collection = $this->_message->getCollection($sourceRefs[$key]->getContent()))) {
				$datastore = $info->appendChild(synd_syncml_node::factory('DataStore'));
				$datastore->appendChild(clone $sourceRefs[$key]);

				$rxPref = $datastore->appendChild(synd_syncml_node::factory('Rx-Pref'));
				$rxPref->appendChild(new synd_syncml_node('CTType', 'text/x-vcalendar'));
				$rxPref->appendChild(new synd_syncml_node('VerCT', '1.0'));
				$datastore->appendChild($rxPref->clone('Rx'));

				$rx = $datastore->appendChild(synd_syncml_node::factory('Rx'));
				$rx->appendChild(new synd_syncml_node('CTType', 'text/plain'));
				$rx->appendChild(new synd_syncml_node('VerCT', '1.0'));
				
				$datastore->appendChild($rxPref->clone('Tx-Pref'));
				$datastore->appendChild($rxPref->clone('Tx'));
				$datastore->appendChild($rx->clone('Tx'));

				$capabilities = SyndLib::array_merge_recursive(
					$capabilities, $collection->getCalendarCapabilities());
			}
		}
		
		$capabilities = SyndLib::array_kintersect($capabilities,
			array_flip(SyndLib::invoke($devinfo->selectNodes('CTCap', 'PropName'), 'getContent')));
		
		if (!empty($capabilities)) {
			array_unshift($capabilities['BEGIN'], 'VCALENDAR');
			array_unshift($capabilities['END'], 'VCALENDAR');
		
			$ctCap = $info->appendChild(synd_syncml_node::factory('CTCap'));
			$ctCap->appendChild(new synd_syncml_node('CTType', 'text/x-vcalendar'));
			
			foreach ($capabilities as $name => $capability) {
				$ctCap->appendChild(new synd_syncml_node('PropName', $name));
				if (is_array($capability)) {
					foreach ($capability as $value) 
						$ctCap->appendChild(new synd_syncml_node('ValEnum', $value));
				}
			}
		}

		$synccap = $info->appendChild(new synd_syncml_node('SyncCap'));
		$synccap->appendChild(new synd_syncml_node('SyncType', 1)); // Two-Way sync
		$synccap->appendChild(new synd_syncml_node('SyncType', 2)); // Slow Two-Way sync
		$synccap->appendChild(new synd_syncml_node('SyncType', 3)); // One-Way sync from client
		$synccap->appendChild(new synd_syncml_node('SyncType', 5)); // One-Way sync from server
		$synccap->appendChild(new synd_syncml_node('SyncType', 6)); // Refresh from server

		return array($info);
	}
	
	function append($memento) {
		$persistent = $this->_message->getPersistentSession();
		$persistent->setDeviceInfo($memento);
		return SyndLib::getInstance('node.case.devinfo');
	}
}

