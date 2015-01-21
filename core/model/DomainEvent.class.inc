<?php
require_once 'core/model/DomainObject.class.inc';

/**
 * Events are triggered upon actions on domain objects
 * @access		public
 * @package		synd.core.model
 */
interface DomainEvent {
	/**
	 * Returns a description
	 * @return	string
	 */
	function __toString();
	
	/**
	 * Returns a longer description
	 * @return	string
	 */
	function getDescription();

	/**
	 * The originating object that raised the event
	 * @return	DomainObject
	 */
	function getSubject();
	
	/**
	 * The interfaces supported by this object
	 * @return	array
	 */
	function getInterfaces();
}

abstract class AbstractDomainEvent implements DomainEvent {
	protected $_subject = null;
	
	function __construct(DomainObject $subject) {
		$this->_subject = $subject;
	}
	
	function getSubject() {
		return $this->_subject;
	}
	
	function getInterfaces() {
		return self::getImplementedInterfaces(new ReflectionObject($this));
	}
	
	static function getImplementedInterfaces(ReflectionClass $class) {
		$interfaces = array($class->getName());
		foreach ($class->getInterfaces() as $interface)
			$interfaces = array_merge($interfaces, self::getImplementedInterfaces($interface));
		if (null != ($parent = $class->getParentClass()))
			$interfaces = array_merge($interfaces, self::getImplementedInterfaces($parent));
		return $interfaces;
	}
}
