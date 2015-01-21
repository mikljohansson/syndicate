<?php
require_once 'core/index/Index.class.inc';

/**
 * Content interface
 * 
 * Content must define these templates
 *  full_view.tpl
 *
 * @abstract
 * @access		public
 * @package		synd.core.model
 */
interface DomainContent extends Indexable {
	/**
	 * Return the text content
	 * @param	string	Text content
	 */
	function toString();

	/**
	 * Set the text content
	 * @param	string	Text content
	 */
	function setContent($content);
	
	/**
	 * Deallocates all resources used by this content
	 */
	function delete();
}
