<?php
require_once 'core/model/DatabaseEntity.class.inc';

/**
 * Inventory instance
 */
class synd_node_instance extends AbstractDatabaseEntity {
	/**
	 * Buffered optional values
	 * @access	private
	 */
	var $_values = null;
	
	function __factory() {
		$this->data['NODE_ID'] = $this->classId().'.'.$this->_db->nextId('synd_instance');
		$this->data['TS_CREATE'] = $this->data['TS_UPDATE'] = time();
		parent::__factory();
	}
	
	function __clone() {
		$this->data['NODE_ID'] = $this->classId().'.'.$this->_db->nextId('synd_instance');
		$this->data['TS_CREATE'] = time();
		parent::__clone();
	}
	
	static function getEntityQuery(DomainStorage $storage) {
		return self::createQuery($storage, 'synd_instance');
	}

	function nodeId() {
		return $this->data['NODE_ID'];
	}
	
	function hasPermission($user, $perm, $arg = null) {
		if ('merge' == $perm && in_array($arg, array(
			'PARENT_NODE_ID','CLASS_NODE_ID')))
			return $this->hasPermission($user, 'write');
		return parent::hasPermission($user, $perm, $arg);
	}

	/**
	 * @return	synd_node_folder
	 */
	function getParent() {
		if (null === ($parent = $this->_storage->getInstance($this->data['PARENT_NODE_ID'])))
			$parent = parent::getParent();
		return $parent;
	}

	function setParent($parent) {
		$this->data['PARENT_NODE_ID'] = $parent->nodeId();
	}
	
	/**
	 * Returns the meta field class if one is set
	 * @return	synd_node_class	Returns synd_node_null on none found
	 */
	function getClass() {
		if (null === ($class = $this->_storage->getInstance($this->data['CLASS_NODE_ID'])))
			$class = SyndNodeLib::getInstance('null.null');
		return $class;
	}
	
	/**
	 * Sets the meta field class
	 * @param	synd_node_class
	 */
	function setClass($class) {
		if ($this->data['CLASS_NODE_ID'] != $class->nodeId) {
			$this->data['CLASS_NODE_ID'] = $class->nodeId;
			$this->_values = null;
		}
	}
	
	/**
	 * Returns meta-class options
	 * @return	array
	 */
	function getClassOptions() {
		$sql = "
			SELECT c.node_id, c.name FROM synd_class c
			ORDER BY c.name";
		return $this->_storage->getInstances($this->_db->getCol($sql));
	}
	
	/**
	 * Returns non-null extra values as array(fieldid => value, ...)
	 * @return	array
	 */
	function getValues() {
		if (null === $this->_values) {
			$class = $this->getClass();
			$this->_values = $class->isNull() ? array() : $class->getValues($this);
		}
		return $this->_values;
	}
	
	/**
	 * Returns recursive meta-field values
	 * @return	array
	 */
	function getOptionalValues() {
		$parent = $this->getParent();
		if ($parent->isNull()) 
			return $this->getValues();
		$values = $parent->getOptionalValues();
		foreach ($this->getValues() as $key => $value)
			$values[$key] = $value;
		return $values;
	}

	/**
	 * Returns recursive meta-field definitions
	 * @return	array
	 */
	function getOptionalDefinitions() {
		$parent = $this->getParent();
		$class = $this->getClass();
		return array_merge(
			$parent->isNull() ? array() : $parent->getOptionalDefinitions(),
			$class->isNull() ? array() : $class->getFields());
	}

	function validate($data) {
		$errors = array();
		if (isset($data['value']) && isset($data['value'][0], $data['value'][1]) && null != $data['value'][1])
			$data['values'][$data['value'][0]] = $data['value'][1];

		if (!empty($data['values'])) {
			if (!isset($data['CLASS_NODE_ID']) || null === ($class = $this->_storage->getInstance($data['CLASS_NODE_ID'])))
				$class = $this->getClass();
			if (!$class->isNull()) {
				$fields = $class->getFields();
				foreach (array_keys($data['values']) as $key) {
					if (isset($fields[$key]) && true !== ($error = $fields[$key]->validate($data['values'][$key])))
						$errors[$key] = $error;
				}
			}
		}

		return array_merge(parent::validate($data), $errors);
	}
	
	function merge($data) {
		if (isset($data['PARENT_NODE_ID']) && null !== ($parent = SyndNodeLib::getInstance($data['PARENT_NODE_ID'])))
			$this->setParent($parent);
		unset($data['PARENT_NODE_ID']);

		if (isset($data['CLASS_NODE_ID']) && null !== ($class = $this->_storage->getInstance($data['CLASS_NODE_ID'])))
			$this->setClass($class);
		unset($data['CLASS_NODE_ID']);

		if (isset($data['value']) && isset($data['value'][0], $data['value'][1]) && null != $data['value'][1])
			$data['values'][$data['value'][0]] = $data['value'][1];
		if (!empty($data['values'])) {
			$class = $this->getClass();
			$this->_values = array();
			
			if (!$class->isNull()) {
				foreach (array_keys($fields = $class->getFields()) as $key) {
					if (isset($data['values'][$key]))
						$this->_values[$key] = $fields[$key]->modify($data['values'][$key]);
				}
			}
		}

		parent::merge($data);
	}
	
	function _entity_onpreflush() {
		parent::_entity_onpreflush();
		$this->data['TS_UPDATE'] = time();
	}

	/**
	 * Stores the extra values
	 */
	function _entity_onpostflush() {
		parent::_entity_onpostflush();
		if (null !== $this->_values) {
			$class = $this->getClass();
			if (!$class->isNull() && !$class->setValues($this, $this->_values))
				return false;
		}
	}
}
