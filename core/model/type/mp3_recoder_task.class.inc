<?php
require_once 'core/model/type/task.class.inc';

class synd_type_mp3_recoder_task extends synd_type_task {
	var $_task_time = 1200;
	var $_lame	= null;
	var $_src 	= null;
	var $_trg 	= null;
	var $_rate 	= null;
	var $_vbr	= null;
	
	function __construct($src, $trg = null, $parm = null) {
		global $synd_config;
		parent::__construct();
		if (null == ($this->_lame = SyndLib::which('lame')))
			trigger_error('lame does not exist or is not executable', E_USER_ERROR);
		$this->_src  = $src;
		$this->_trg  = (null != $trg ? $trg : $this->_src);
		$this->_rate = (isset($parm['rate']) ? $parm['rate'] : 128);
		$this->_vbr  = (isset($parm['vbr']) ? $parm['vbr'] : false);
		$this->_comment = tpl_request_host();
	}
	
	function run() {
		parent::run();
		$title = str_replace('_',' ',basename($this->_src, '.'.SyndLib::fileExtension($this->_src)));
		
		if (@include_once 'MP3/Id.php') {
			$id3tag = new MP3_Id();
			$id3tag->read($this->_src);
			
			$title  = $id3tag->getTag('name', $title);
			$artist = $id3tag->getTag('artists', null);
			$album  = $id3tag->getTag('album', null);
			$year   = $id3tag->getTag('year', null);
			$track  = $id3tag->getTag('track', null);
			$genre  = $id3tag->getTag('genre', null);
		}
		
		$id3 = "--tc '{$this->_comment}' --tt '$title' --ta '$artist' --tl '$album' --ty '$year' --tn '$track' --tg '$genre'";
		$cmd = "{$this->_lame} -t -q 4 -b {$this->_rate} --silent $id3";

		if ('mp3' == SyndLib::fileExtension($this->_src))
			$cmd .= ' --mp3input';
		
		// Set maximum bitrate to one step over medium bitrate
		if ($this->_vbr) {
			$rate = explode(' ', "8 16 24 32 40 48 56 64 80 96 112 128 160");
			$cmd .= ' -v -B '.($rate[array_search($this->_rate, $rate)+1]);
		}
			
		$cmd .= ' '.escapeshellarg($this->_src).' '.escapeshellarg($this->_trg);
		print "Command: $cmd\n";
		passthru($cmd);
	}
}
