<?php
require_once 'core/model/type/file.class.inc';

class synd_type_mp3 extends synd_type_file {
	function getResizedUri($rate) {
		global $synd_config;
		return $synd_config['dirs']['var']['uri'].$this->resizedFile($rate);
	}

	function resizedPath($rate) {
		global $synd_config;
		return $synd_config['dirs']['var']['path'].$this->resizedFile($rate);
	}

	function resizedFile($rate) {
		global $synd_config;
		$file = SyndLib::chopExtension($this->_file)."_{$rate}kbps.".SyndLib::fileExtension($this->_file);
		$dest = $synd_config['dirs']['var']['path'].$file;

		if (!file_exists($dest)) 
			$this->copyResized($dest, array('rate'=>$rate));
		
		return $file;
	}

	function copyResized($dest, $param) {
		if (!isset($param['vbr']))
			$param['vbr'] = true;
		
		touch($dest);

		require_once 'core/lib/SyndTaskHandler.class.inc';
		$handler = SyndTaskHandler::factory('mp3');
		$handler->appendTask(SyndType::factory('mp3_recoder_task', $this->path(), $dest, $param), 
			md5($this->path()), 1200);
	}
}
