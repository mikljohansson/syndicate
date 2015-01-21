<?php

/**
 * Menu builder interface
 * @access		public
 * @package		synd.design
 */
interface MenuBuilder {
	/**
	 * Output the menu
	 */
	function display();

	/**
	 * Append a menu item to this menu
	 * @param	string	Uri of item
	 * @param	string	Link text of item
	 * @param	string	Link title (title="" attribute)
	 * @param	string	Submenu callback
	 * @param	array	List of patterns to expand submenu, see fnmatch()
	 * @param	string	Access key for "Alt+X" style shortcuts
	 */
	function append($uri, $text, $title = '', $callback = null, $patterns = null, $accesskey = null);
	
	/**
	 * Does the menu have any items
	 * @return	bool
	 */
	function hasChildren();
}

/**
 * Renders menu using <ul class="Menu"><li><a ...
 */
class ListMenu implements MenuBuilder {
	protected $_path = null;
	protected $_menu = null;
	
	/**
	 * @param	string	Path to expand
	 * @param	integer	Menu section to render
	 */
	function __construct($path = '') {
		$this->_path = $path;
	}
	
	function display() {
		if (null != $this->_menu) {
			print '<ul class="Menu">';
			print $this->_menu;
			print '</ul>';
		}
	}
	
	function hasChildren() {
		return null != $this->_menu;
	}
	
	function append($uri, $text, $title = '', $callback = null, $patterns = null, $accesskey = null) {
		assert('null === $callback || is_callable($callback)');
		
		// Expand submenu if needed
		if (null == $callback) 
			$class = 'Leaf';
		else {
			$class = 'Collapsed';
			$expand = false;

			if (empty($patterns)) {
				$expand = true;
				$class = 'Expanded';
			}
			else {
				foreach ($patterns as $pattern) {
					if (fnmatch($pattern, $this->_path)) {
						$expand = true;
						$class = 'Expanded';
						break;
					}
				}
			}
		}
		
		if (null == $uri)
			$this->_menu .= "<li class=\"$class\">$text";
		else {
			$this->_menu .= "<li class=\"$class\"><a href=\"$uri\" title=\"$title\"";
			if (null != $accesskey)
				$this->_menu .= " accesskey=\"$accesskey\"";
			$this->_menu .= ">$text</a>";
		}
		
		if (null != $callback && $expand) {
			$this->_menu .= '<ul>';
			call_user_func($callback, $this);
			$this->_menu .= '</ul>';
		}
		
		$this->_menu .= '</li>';
	}
}

/**
 * Counts the items in a menu
 */
class IndicatorMenu implements MenuBuilder {
	protected $_count = 0;
	
	function display() {}
	
	function append($uri, $text, $title = '', $callback = null, $patterns = null, $accesskey = null) {
		$this->_count++;
	}
	
	function hasChildren() {
		return (bool)$this->_count;
	}
}