<?php
require_once 'core/model/DomainContent.class.inc';

class synd_type_html extends SyndType implements DomainContent {
	var $_text = null;
	var $_list = null;

	function toString() {
		return $this->_text;
	}
	
	function setContent($content) {
		$this->importText($content);
	}

	function __clone() {
		parent::__clone();

		if (count($this->_list)) {
			$list = array();
			$text = $this->_text;
			
			foreach (array_keys($this->_list) as $key) {
				$clone = $this->_list[$key]->copy();
				$list[] = $clone;

				$uri = SyndLib::chopExtension($this->_list[$key]->uri());
				$uri = str_replace('/', '\/', $uri);
				$text = preg_replace('/'.$uri.'[^"]*/', $clone->uri(), $text);
			}
			
			$this->_list = $list;
			$this->importText($text);
		}
	}

	function importText($text) {
		require_once 'core/lib/SyndHTML.class.inc';
		$text = SyndHTML::tidyHtml($text);
		$text = SyndHTML::filterHtml($text);

		$this->_text = $text;
		
		if (null != $this->_list) {
			foreach (array_keys($this->_list) as $key) {
				// Delete files not used anymore
				$oid = $this->_list[$key]->objectId();
				if (false === strpos($this->_text, $oid)) {
					$this->_list[$key]->delete();
					unset($this->_list[$key]);
					continue;
				}

				// Resize images
				if (($this->_list[$key] instanceof synd_type_image)) {
					// Find instances of this image
					$oid = preg_quote($this->_list[$key]->objectId(), '/');
					preg_match_all('/<img\s[^>]*src="[^"]*'.$oid.'[^"]*"[^>]*>/i', $this->_text, $imgTags);

					foreach ($imgTags[0] as $imgTag) {
						if (!preg_match('/style="[^"]*width:\s*(\d+)px/is', $imgTag, $width))
							preg_match('/width="?(\d+)"?/i', $imgTag, $width);
						if (!preg_match('/style="[^"]*height:\s*(\d+)px/is', $imgTag, $height))
							preg_match('/height="?(\d+)"?/i', $imgTag, $height);

						$width = isset($width[1]) ? $width[1] : null;
						$height = isset($height[1]) ? $height[1] : null;

						if ($width < $this->_list[$key]->getWidth() || $height < $this->_list[$key]->getHeight())
							$uri = $this->_list[$key]->getResizedUri($width, $height);
						else
							$uri = $this->_list[$key]->uri();
							
						$newTag = preg_replace('/src="[^"]+"/i', 'src="'.$uri.'"', $imgTag);
						$this->_text = str_replace($imgTag, $newTag, $this->_text);
					}
				}
			}
		}
	}

	function delete() {
		if (is_array($this->_list)) {
			foreach (array_keys($this->_list) as $key)
				$this->_list[$key]->delete();
		}
		parent::delete();
	}

	function getSize() {
		$size = 0;
		if (is_array($this->_list)) {
			foreach (array_keys($this->_list) as $key)
				$size += $this->_list[$key]->getSize();
		}
		return $size+strlen($this->_text);
	}

	/**
	 * Implementation of the SyndRTE upload interface
	 * @param	array
	 * @return	array
	 */
	function _view_upload(Request $request, Template $page) {
		if (null == $_FILES['image']['tmp_name'] || null == $_FILES['image']['name']) 
			$error = SyndLib::translate('No image recieved');
		else {
			$tmpLocation = $_FILES['image']['tmp_name'];
			$info = pathinfo($_FILES['image']['name']);
			if (!preg_match('/^(jpg|jpeg|gif|png|mng)$/i', $info['extension']) || !is_uploaded_file($tmpLocation))
				$error = SyndLib::translate('Invalid or unsupported image extension: %s', $info['extension']);
			else if (null !== ($image = SyndType::factory('image', $tmpLocation, $_FILES['image']['name']))) {
				if ($image->getHeight()) {
					$this->_list[] = $image;
					if (null != ($parent = $this->getParent())) {
						$parent->_save();
						$image->setParent($parent);
					}
				}
				else {
					$error = SyndLib::translate('Invalid or unsupported image: %s', $_FILES['image']['name']);
				}
			}
		}

		$redirect = $_REQUEST['redirect'];
		$redirect .= false === strpos($redirect, '?') ? '?' : '&';
		$redirect .= isset($error) ? 'error='.rawurlencode($error) : 'image='.rawurlencode($image->uri());
		return array('redirect' => $redirect);
	}

	function acceptIndexer(IndexBuilder $indexer) {
		$indexer->visitFragment($this->_text);
	}
}
