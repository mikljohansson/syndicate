<?php
require_once 'core/model/type/file.class.inc';

class synd_type_movie extends synd_type_file {
	var $_thumb = null;
	
	function delete() {
		parent::delete();
		if (null != $this->_thumb)
			$this->_thumb->delete();
	}
	
	function getImage() {
		return $this->_thumb;
	}

	/**
	 * Regenerates the movie thumb image.
	 */
	function regenerateThumb() {
		require_once 'core/lib/SyndTaskHandler.class.inc';
		$handler = SyndTaskHandler::factory('movie');
		$handler->appendTask(SyndType::factory('movie_recoder_task', $this), 
			md5($this->path()), 1200);
	}
	
	/**
	 * Imports an image to use as thumb.
	 * @see		synd_type_movie_recode_task
	 * @param	string	Path to thumb
	 */
	function importThumb($thumb) {
		if (null != $this->_thumb)
			$this->_thumb->delete();
		
		$this->_thumb = SyndType::factory('image', $thumb);
		$this->_thumb->setParent($this->getParent());

		if (null != ($parent = $this->getParent()))
			$parent->_save();
	}

	function resizedThumbUri($x = null, $y = null) {
		if (null != $this->_thumb)
			return $this->_thumb->getResizedUri($x, $y);
		return null;
	}
}
