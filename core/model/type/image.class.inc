<?php
require_once 'core/model/type/file.class.inc';

class synd_type_image extends synd_type_file {
	/**
	 * @access	private
	 */
	var $_width = null;

	/**
	 * @access	private
	 */
	var $_height = null;

	function getWidth() {
		if (null === $this->_width && is_array($size = @getimagesize($this->path()))) {
			$this->_width = $size[0];
			$this->_height = $size[1];
		}
		return $this->_width;
	}
	
	function getHeight() {
		if (null === $this->_height && is_array($size = @getimagesize($this->path()))) {
			$this->_width = $size[0];
			$this->_height = $size[1];
		}
		return $this->_height;
	}
	
	/**
	 * Removes all cached thumbnails
	 */
	function delete() {
		SyndLib::runHook('lob_storage_delete', SyndLib::chopExtension($this->_file), true);
		return parent::delete();
	}
	
	function getResizedUri($width = null, $height = null) {
		if (null == $height && null != $width)
			$height = round($width / ($this->getWidth()/$this->getHeight()));
		else if (null == $width && null != $height)
			$width = round($height * ($this->getWidth()/$this->getHeight()));
		else if (null == $width && null == $height)
			return $this->uri();

		if ('gif' == ($type = strtolower(SyndLib::fileExtension($this->_file)))) 
			$type = 'png';
		$target = SyndLib::chopExtension($this->_file)."_{$width}x{$height}.$type";
		
		// Check for cached copy
		if (null != ($uri = SyndLib::runHook('lob_storage_uri', $target, true)))
			return $uri;

		// Read source image into buffer
		if (false == ($path = $this->path()) || false == ($srcBuffer = $this->_read($path)))
			return $file;
				
		// Create destination buffer
		$trgBuffer = function_exists('imagecreatetruecolor') ? 
			imagecreatetruecolor($width, $height) : imagecreate($width, $height);
		if (false == $trgBuffer)
			return false;

		// Copy and resize buffer
		if (!imagecopyresized($trgBuffer, $srcBuffer, 0,0,0,0, $width, $height, $this->getWidth(), $this->getHeight()))
			return false;

		// Write buffer to storage
		return $this->_write($target, $trgBuffer);
	}

	/**
	 * Returns an URI to an image no larger than the specified size
	 * @param	integer	Maximum width to allow
	 * @param	integer	Maximum height to allow
	 * @return	string
	 */
	function getBoxedUri($width = null, $height = null) {
		$tw = $this->getWidth();
		$th = $this->getHeight();
	
		if ($tw > $th) {
			if (null != $width && $tw > $width) {
				$th = round($width * ($th / $tw));
				$tw = $width;
			}
		}
		else {
			if (null != $height && $th > $height) {
				$tw = round($height * ($tw / $th));
				$th = $height;
			}
		}

		if ($this->getWidth() != $tw || $this->getHeight() != $th)
			return $this->getResizedUri($tw, $th);
		
		return $this->uri();
	}
	
	static function nullInstance() {
		return new _synd_type_image_null();
	}

	function rotate($degrees = 90) {
		if (false != ($srcBuffer = $this->_read($this->path())) &&
			false != ($trgBuffer = imagerotate($srcBuffer, -$degrees, 0xFFFFFF)) &&
			false != ($uri = $this->_write($this->_file, $trgBuffer))) {
			$this->_width = null; $this->getWidth();
			$this->_height = null; $this->getHeight();
			return $uri;
		}
		return false;
	}

	/**
	 * Read the image into a raw buffer
	 * @access	private
	 * @param	string		Source image path
	 * @return	resource
	 */
	function _read($path) {
		switch (strtolower(SyndLib::fileExtension($path))) {
			case 'jpe':
			case 'jpg':
			case 'jpeg':
				return @imagecreatefromjpeg($path);
			case 'png':
				return @imagecreatefrompng($path);
			case 'gif':
				if (function_exists('imagecreatefromgif'))
					return @imagecreatefromgif($path);
				break;
			case 'wbmp':
				return @imagecreatefromwbmp($path);
		}
		return false;
	}

	/**
	 * Transforms the raw buffer into a formatted image buffer
	 * @access	private
	 * @param	string		Target image name
	 * @param	resource	Raw image buffer
	 * @param	integer		Output quality (0-100)
	 * @return	string
	 */
	function _write($target, $buffer, $quality = 60) {
		ob_start();
		
		switch (strtolower(SyndLib::fileExtension($target))) {
			case 'jpe':
			case 'jpg':
			case 'jpeg':
				$result = imagejpeg($buffer, null, $quality);
				break;
			case 'gif':
			case 'png':
				$result = imagepng($buffer);
				break;
			case 'wbmp':
				$result = imagewbmp($buffer);
				break;
			default:
				$result = false;
		}
		
		if ($result && null != ($buffer = ob_get_clean()))
			return SyndLib::runHook('lob_storage_put', $target, $buffer, 1);
		
		ob_end_clean();
		return false;
	}
}

/**
 * @access	private
 */
class _synd_type_image_null extends synd_type_image {
	var $_file = null;
	
	function __construct() {
		$this->_file = tpl_design_uri('image/notfound.jpg');
	}
	
	function isNull() {
		return true;
	}

	function path() {
		return tpl_design_path('image/notfound.jpg');
	}

	function uri() {
		return tpl_design_uri('image/notfound.jpg');
	}
}
