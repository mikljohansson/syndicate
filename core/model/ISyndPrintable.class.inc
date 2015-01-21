<?php

/**
 * Printable interface
 * @abstract
 * @access		public
 * @package		synd.core.model
 */
class ISyndPrintable {
	/**
	 * Prints this object
	 *
	 * Returns true on successful printing
	 *
	 * @param	string	Name of printer (laserprinter1@printserver.example.com, ...)
	 * @return	bool
	 */
	function toPrinter($printer) {}
}