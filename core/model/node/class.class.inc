<?php
require_once 'core/model/DatabaseEntity.class.inc';

class synd_node_class extends AbstractDatabaseEntity {
	/**
	 * Cached meta field info
	 * @access	private
	 */
	var $_fields = null;
	
	function __factory() {
		$this->data['NODE_ID'] = $this->classId().'.'.$this->_db->nextId('synd_class');
		parent::__factory();
	}

	function __clone() {
		$this->data['NODE_ID'] = $this->classId().'.'.$this->_db->nextId('synd_class');
		parent::__clone();
	}
	
	static function getEntityQuery(DomainStorage $storage) {
		return self::createQuery($storage, 'synd_class');
	}

	function nodeId() {
		return $this->data['NODE_ID'];
	}
	
	function getHandler() {
		return 'inventory';
	}
	
	function hasPermission($user, $perm, $arg = null) {
		$inventory = Module::getInstance('inventory');
		if ('read' == $perm && $inventory->hasPermission($user, 'read'))
			return true;
		if ('write' == $perm && $inventory->hasPermission($user, 'admin'))
			return true;
		if ('merge' == $perm && in_array($arg, array('NAME','INFO_BODY')))
			return $this->hasPermission($user, 'write');
		return parent::hasPermission($user, $perm, $arg);
	}

	function isLeaseOnly() {
		return $this->data['FLAG_LEASE_ONLY'];
	}
	
	function toString() {
		return $this->data['NAME'];
	}
	
	function setTitle($title) {
		$this->data['NAME'] = $title;
		$this->_save();
	}
	
	function getItemCount() {
		$sql = "
			SELECT COUNT(1) FROM synd_instance i
			WHERE i.class_node_id = ".$this->_db->quote($this->nodeId);
		return $this->_db->getOne($sql);
	}
	
	function getNumericFields() {
		$sql = "
			SELECT f.field_id, f.info_head 
			FROM synd_class_fields f
			WHERE 
				f.class_node_id = '$this->nodeId' AND
				f.info_datatype = 'numeric'
			ORDER BY f.info_head";
		return $this->_db->getAssoc($sql);
	}

	/**
	 * Instantiate a datatype
	 * @param	string	Datatype {string, numeric, mac}
	 * @param	string	Name of field
	 * @return	ISyndDatatype
	 */
	static function type($type, $field = null) {
		switch ($type) {
			case 'numeric':
				return new SyndNumericDatatype($field);
			case 'date':
				return new SyndDateDatatype($field);
			case 'mac':
				return new SyndMacAddressDatatype($field);
			case 'ip':
				return new SyndIpAddressDatatype($field);
		}
		
		if (null !== ($result = SyndLib::runHook('type', $type, $field)))
			return $result;
		return new SyndStringDatatype($field);
	}

	function getDatatypes() {
		$types = array(
			'string'	=> new SyndStringDatatype(),
			'numeric'	=> new SyndNumericDatatype(),
			'date'		=> new SyndDateDatatype(),
			'mac'		=> new SyndMacAddressDatatype(),
			'ip'		=> new SyndIpAddressDatatype());
		return array_merge($types, (array)SyndLib::runHook('types'));
	}

	/**
	 * Returns meta fields
	 *
	 * Returns an array of fields and datatypes
	 *  array(name => object SyndDatatype, ...)
	 *
	 * @return	array
	 */
	function getFields() {
		$fields = array();
		foreach ($this->_getFieldDefinitions() as $id => $definition)
			$fields[$id] = synd_node_class::type($definition['INFO_DATATYPE'], $definition['INFO_HEAD']);
		return $fields;
	}
	
	/**
	 * Returns field data indexed on FIELD_ID
	 * @access	private
	 */
	function _getFieldDefinitions() {
		if (null === $this->_fields) {
			$sql = "
				SELECT f.* FROM synd_class_fields f
				WHERE f.class_node_id = ".$this->_db->quote($this->nodeId)."
				ORDER BY f.info_head";
			$this->_fields = array();
			foreach ((array)$this->_db->getAll($sql) as $field)
				$this->_fields[$field['FIELD_ID']] = $field;
			$this->_node_onchange_internal();
		}
		return $this->_fields;
	}
	
	/**
	 * Adds a new field to this class
	 * @param	string	Name of field
	 * @param	string	Datatype of field {string, numeric, mac}
	 */
	function addField($name, $type) {
		if (null === $this->_fields)
			$this->getFields();

		foreach (array_keys($this->_fields) as $id) {
			if ($this->_fields[$id]['INFO_HEAD'] == $name) {
				$this->_fields[$id]['INFO_DATATYPE'] = $type;
				$this->_node_onchange_internal();
				return;
			}
		}
		
		$id = $this->_db->nextId('synd_class_fields');
		$this->_fields[$id] = array(
			'FIELD_ID'		=> $id,
			'CLASS_NODE_ID'	=> $this->nodeId,
			'INFO_HEAD'		=> $name,
			'INFO_DATATYPE'	=> $type);
		$this->_node_onchange_internal();
	}
	
	/**
	 * Returns non-null extra values as array(fieldid => value, ...)
	 * @param	synd_node_instance	Instance to get values for
	 * @return	array
	 */
	function getValues($node) {
		$sql = "
			SELECT v.field_id, v.value
			FROM synd_class_fields f, synd_class_values v
			WHERE 
				f.field_id = v.field_id AND
				f.class_node_id = ".$this->_db->quote($this->nodeId)." AND
				v.node_id = ".$this->_db->quote($node->nodeId);
		return $this->_db->getAssoc($sql);
	}
	
	/**
	 * Insert/update values
	 * @param	synd_node_instance	Instance to update for
	 * @param	array				Values to replace as array(fieldid => value, ...)
	 * @return	bool				Return true on success, false on failure
	 */
	function setValues($node, $values) {
		$delete = null;

		foreach ($values as $id => $value) {
			if (null == $value)
				$delete[] = $id;
			else {
				$result = $this->_db->replace('synd_class_values', array(
					'NODE_ID'	=> $node->nodeId,
					'FIELD_ID'	=> $id,
					'VALUE'		=> $value));
				if (SyndLib::isError($result))
					return false;
			}
		}

		if (!empty($delete)) {
			$sql = "
				DELETE FROM synd_class_values
				WHERE 
					node_id = ".$this->_db->quote($node->nodeId)." AND
					field_id IN (".implode(',',$this->_db->quote($delete)).")";
			if (SyndLib::isError($this->_db->query($sql)))
				return false;
		}
		
		return true;
	}
	
	function merge($data) {
		if (isset($data['field'])) {
			if (!empty($data['field']['INFO_HEAD']) && !empty($data['field']['INFO_DATATYPE']))
				$this->addField($data['field']['INFO_HEAD'], $data['field']['INFO_DATATYPE']);
			unset($data['fields']);
		}
		$data['FLAG_LEASE_ONLY'] = (int)!empty($data['FLAG_LEASE_ONLY']);
		parent::merge($data);
	}
	
	function _view_delField(Request $request, Template $page) {
		if (!$this->isPermitted('write'))
			throw new ForbiddenException();
		if (!isset($request['confirm']))
			return array('content' => $page->fetchnode($this, 'confirm_delete_field.tpl', array('field'=>$request[2])));
				
		$sql = "
			DELETE FROM synd_class_fields
			WHERE 
				class_node_id = ".$this->_db->quote($this->nodeId)." AND
				field_id = ".$this->_db->quote($request[2]);
		$this->_db->query($sql);
		
		if (isset($this->_fields[$request[2]])) {
			unset($this->_fields[$request[2]]);
			$this->_node_onchange_internal();
		}
	}
	
	function _entity_onpostflush() {
		parent::_entity_onpostflush();
		if (null !== $this->_fields) {
			foreach ($this->_fields as $definitions)
				$this->_db->replace('synd_class_fields', $definitions);
		}
	}
}

/**
 * Datatype interface
 * @access		public
 * @package		synd.core.model
 */
interface ISyndDatatype {
	/**
	 * Returns the name of this class
	 * @return	string
	 */
	function getDatatypeName();

	/**
	 * Checks if this field implements an interface
	 * @param	string	The interface to check
	 * @return	bool	True on success, false on failure
	 */
	function isInheritedFrom($interface);
	
	/**
	 * Returns the field name
	 * @return	string
	 */
	function toString();
	
	/**
	 * Validates if a value matches this datatype
	 * @param	mixed	Value to check
	 * @return	bool	True on success, false on failure
	 */
	function validate($value);
	
	/**
	 * Modifies a value before write to storage
	 * @param	mixed	Value to check
	 * @return	mixed	Optionally modified value
	 */
	function modify($value);

	/**
	 * Quotes a value for use in config files
	 * @param	mixed	Value to quote
	 * @return	string
	 */
	function quote($value);
}

abstract class SyndDatatype implements ISyndDatatype {
	/**
	 * @access	protected
	 */
	var $_field = null;
	
	/**
	 * @param	string	Name of field
	 */
	function __construct($field = null) {
		$this->_field = $field;
	}
	
	function toString() {
		return $this->_field;
	}
	
	function __toString() {
		return $this->toString();
	}

	function isInheritedFrom($interface) {
		return $this instanceof $interface;
	}
	
	function modify($value) {
		return $value;
	}

	function quote($value) {
		return $value;
	}
}

class SyndNumericDatatype extends SyndDatatype {
	function getDatatypeName() {
		return SyndLib::translate('Numeric');
	}
	
	function validate($value) {
		if (null != $value && !is_numeric($value))
			return SyndLib::translate("Invalid number '%s'", tpl_chop($value,16));
		return true;
	}
}

class SyndStringDatatype extends SyndDatatype {
	function getDatatypeName() {
		return SyndLib::translate('String');
	}
	
	function validate($value) {
		if (null != $value) {
			if (!is_scalar($value))
				return SyndLib::translate("Value is not a scalar type '%s'", gettype($value));
			if (preg_match('/[\r\n\0]/', $value))
				return SyndLib::translate("Value must not contain newlines or NUL");
		}			
		return true;
	}

	function quote($value) {
		return '"'.strtr($value,"\r\n\0", "   ").'"';
	}
}

class SyndDateDatatype extends SyndDatatype {
	function getDatatypeName() {
		return SyndLib::translate('Date');
	}
	
	function validate($value) {
		if (null != $value && !strtotime($value))
			return SyndLib::translate("Invalid date '%s'", tpl_chop($value,10));
		return true;
	}
}

class SyndMacAddressDatatype extends SyndStringDatatype {
	function getDatatypeName() {
		return SyndLib::translate('MAC-address');
	}
	
	function validate($value) {
		if (null != $value && 12 != strlen(preg_replace('/[^0-9a-f]/i', '', $value)))
			return SyndLib::translate("Invalid characters in MAC-address '%s'", tpl_chop($value,16));
		return true;
	}
	
	function modify($value) {
		return strtoupper(preg_replace('/\w{2}(?=\w)/', '\0:', preg_replace('/[^0-9a-f]/i', '', $value)));
	}
}

class SyndIpAddressDatatype extends SyndStringDatatype {
	function getDatatypeName() {
		return SyndLib::translate('IP-address');
	}
	
	function validate($value) {
		if (null != $value) {
			if (!preg_match('/\d{1,3}(\.\d{1,3}){3}/', $value))
				return SyndLib::translate("Invalid IP-address '%s'", tpl_chop($value,16));
			foreach (explode('.', $value) as $part) {
				if ($part > 255)
					return SyndLib::translate("Invalid IP-address '%s'", tpl_chop($value,16));
			}
		}
		return true;
	}
}
