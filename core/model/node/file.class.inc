<?php
require_once 'core/model/node/node.class.inc';
require_once 'core/model/type/file.class.inc';
require_once 'core/model/type/image.class.inc';

class synd_node_file extends synd_node_node {
	function __construct($storage, $database, $data) {
		if (null != $data['DATA_FILE'])
			$data['DATA_FILE'] = unserialize($data['DATA_FILE']);
		if (null != $data['DATA_IMAGE'])
			$data['DATA_IMAGE'] = unserialize($data['DATA_IMAGE']);
		parent::__construct($storage, $database, $data);
	}
	
	static function getEntityQuery(DomainStorage $storage) {
		$query = parent::getEntityQuery($storage);
		$query->join('synd_node_file');
		return $query;
	}

	function toString() {
		$title = trim($this->data['INFO_HEAD']);
		return empty($title) ? SyndLib::translate('No title') : $title;
	}
	
	function _public_toString() {
		return $this->toString();
	}

	function hasPermission($user, $perm, $arg = null) {
		if ('read' == $perm && null == $arg)
			return true;
		$creator = $this->getCreator();
		if ('write' == $perm && $creator->hasAuthorization($user, $this))
			return true;
		if ('append' == $perm && ($arg instanceof synd_node_comment))
			return true;
		if ('merge' == $perm && in_array($arg, array(
			'INFO_WEIGHT','INFO_HEAD','INFO_DESC','DATA_FILE','DATA_IMAGE')))
			return $this->hasPermission($user, 'write');
		return parent::hasPermission($user, $perm, $arg);
	}

	function delete() {
		if (is_object($this->data['DATA_FILE']))
			$this->data['DATA_FILE']->delete();
		parent::delete();
	}
	
	function getDescription() {
		return $this->data['INFO_DESC'];
	}
	
	function getHandler() {
		$parent = $this->getParent();
		return $parent->getHandler();
	}
	
	function getCreator() {
		if (null === ($creator = SyndNodeLib::getInstance($this->data['CREATE_NODE_ID'])))
			$creator = SyndNodeLib::getInstance('null.null');
		return $creator;
	}
	
	function getFile() {
		if (null != $this->data['DATA_FILE'])
			return $this->data['DATA_FILE'];
		return synd_type_file::nullInstance();
	}

	function import($path, $name = null) {
		if (null !== ($file = SyndType::factory('file', $path, $name))) {
			if (is_object($this->data['DATA_FILE']))
				$this->data['DATA_FILE']->delete();
			$file->setParent($this);
			$this->data['DATA_FILE'] = $file;
		}
		if (empty($this->data['INFO_HEAD']))
			$this->data['INFO_HEAD'] = SyndLib::chopExtension($file->toString());
		$this->_save();
	}
	
	function hasImage() {
		return null != $this->data['DATA_IMAGE'];
	}
	
	function getImage() {
		if (null != $this->data['DATA_IMAGE'])
			return $this->data['DATA_IMAGE'];
		return synd_type_image::nullInstance();
	}
	
	function getWeight() {
		return $this->data['INFO_WEIGHT'];
	}

	function setWeight($weight) {
		$this->data['INFO_WEIGHT'] = $weight;
		$this->_save();
	}
	
	function path() {
		$file = $this->getFile();
		return $file->path();
	}

	function getIndexDocument($index) {
		if (!$this->isPublished())
			return null;
		return $index->createComposite(array(
			$index->createFragment($this->data['INFO_HEAD']),
			$index->createFragment($this->data['INFO_DESC'], 0.8)));
	}

	function validate($data) {
		require_once 'core/lib/SyndHTML.class.inc';
		$errors = parent::validate($data);
		SyndHTML::validateFile($data, $errors, 'DATA_FILE');
		SyndHTML::validateFile($data, $errors, 'DATA_IMAGE');
		return $errors;
	}

	function merge($data) {
		require_once 'core/lib/SyndHTML.class.inc';

		if (!empty($data['DATA_FILE']['tmp_name']) &&
			null !== ($file = SyndType::factory('file', $data['DATA_FILE']['tmp_name'], $data['DATA_FILE']['name']))) {
			if (is_object($this->data['DATA_FILE']))
				$this->data['DATA_FILE']->delete();
			$file->setParent($this);
			$this->data['DATA_FILE'] = $file;
			if (empty($data['INFO_HEAD']) && empty($this->data['INFO_HEAD']))
				$data['INFO_HEAD'] = SyndLib::chopExtension($file->toString());
		}
		unset($data['DATA_FILE']);

		if (!empty($data['DATA_IMAGE']['tmp_name']) &&
			null !== ($file = SyndType::factory('image', $data['DATA_IMAGE']['tmp_name'], $data['DATA_IMAGE']['name']))) {
			if (is_object($this->data['DATA_IMAGE']))
				$this->data['DATA_IMAGE']->delete();
			$file->setParent($this);
			$this->data['DATA_IMAGE'] = $file;

		}
		unset($data['DATA_IMAGE']);
		
		if (isset($data['INFO_DESC']))
			$data['INFO_DESC'] = SyndHTML::filterHtml(SyndHTML::tidyHtml($data['INFO_DESC']));
		if (isset($data['INFO_BODY']))
			$data['INFO_BODY'] = SyndHTML::filterHtml(SyndHTML::tidyHtml($data['INFO_BODY']));
		
		parent::merge($data);
	}

	function _view_removeImage(Request $request, Template $page) {
		if (!$this->isPermitted('write'))
			throw new ForbiddenException();
		$this->data['DATA_IMAGE'] = null;
		$this->_save();
	}
}
