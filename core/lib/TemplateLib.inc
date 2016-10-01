<?php
require_once 'core/lib/SyndLib.class.inc';

/**
 * Template utility functions
 * @access		public
 * @package		synd.core.lib
 * @filesource
 */

/**
 * @access	private
 */
function _tpl_view_rewrite($args, $stack = null, $separator = '&') {
	global $synd_config;
	$path = empty($synd_config['rewrite']['enable']) ? 
		$synd_config['dirs']['root']['uri'].'index.php?sq=' : 
		$synd_config['rewrite']['base'];
	$query = null;

	for ($i=0, $c=count($args); $i<$c; $i++) {
		if (is_array($args[$i])) {
			$query = $args[$i];
			break;
		}

		$path .= rawurlencode($args[$i]);
		$path .= '/';
	}
	
	if (null != $stack)
		$query['stack'] = $stack;
	if (null != $query) {
		$path .= empty($synd_config['rewrite']['enable']) ? $separator : '?';
		$path .= http_build_query($query, false, $separator);
	}

	return $path;
}

/**
 * Returns an uri to a module view.
 * @param	mixed,...	Uri components or parameters
 * @return	string
 */
function tpl_view() {
	return _tpl_view_rewrite(func_get_args());
}

/**
 * Returns an uri with the current view prepended to the callstack
 * @param	mixed,...	Uri components or parameters
 * @return	string
 */
function tpl_view_call() {
	$args = func_get_args();
	$stack = isset($_REQUEST['stack']) ? $_REQUEST['stack'] : array();
	$stack[] = rtrim(preg_replace('/&?stack(\[|%5B)\d*(\]|%5D)=[^&]*/i', '', 
		isset($args[2], $args[3]) && (is_null($args[2]) || is_array($args[2])) && is_string($args[3]) ? $args[3] : 
		$_SERVER['REQUEST_URI']), '?');
	return _tpl_view_rewrite($args, $stack);
}

/**
 * Returns an uri with the current callstack preserved
 * @param	mixed,...	Uri components or parameters
 * @return	string
 */
function tpl_view_jump() {
	$args = func_get_args();
	$stack = isset($_REQUEST['stack']) ? $_REQUEST['stack'] : array($_SERVER['REQUEST_URI']);
	return _tpl_view_rewrite($args, $stack);
}

/**
 * For use in <a href=""> (uses &amp; for separator)
 * @see	tpl_view()
 */
function tpl_link() {
	return _tpl_view_rewrite(func_get_args(), null, '&amp;');
}

/**
 * For use in <a href=""> (uses &amp; for separator)
 * @see	tpl_view_call()
 */
function tpl_link_call() {
	$args = func_get_args();
	$stack = isset($_REQUEST['stack']) ? $_REQUEST['stack'] : array();
	$stack[] = rtrim(preg_replace('/&?stack(\[|%5B)\d*(\]|%5D)=[^&]*/i', '', 
		isset($args[2], $args[3]) && (is_null($args[2]) || is_array($args[2])) && is_string($args[3]) ? $args[3] : 
		$_SERVER['REQUEST_URI']), '?');
	return _tpl_view_rewrite($args, $stack, '&amp;');
}

/**
 * For use in <a href=""> (uses &amp; for separator)
 * @see	tpl_view_jump()
 */
function tpl_link_jump($module, $view = null, $args = null) {
	$args = func_get_args();
	$stack = isset($_REQUEST['stack']) ? $_REQUEST['stack'] : array($_SERVER['REQUEST_URI']);
	return _tpl_view_rewrite($args, $stack, '&amp;');
}


function tpl_uri($query = null, $script = null, $separator = '&') {
	if (null == $script)
		$script = isset($_SERVER['REDIRECT_URL']) ? $_SERVER['REDIRECT_URL'] : $_SERVER['SCRIPT_NAME'];
	if (!empty($query))
		return $script.'?'.http_build_query($query, false, $separator);
	return $script;
}

function tpl_uri_merge($query = null, $script = null, $separator = '&') {
	$get = $_GET;
	if (isset($get['sq']))
		unset($get['sq']);
	return tpl_uri(SyndLib::array_merge_recursive($get, $query), $script, $separator);
}

function tpl_uri_jump($query = null, $script = null, $separator = '&') {
	$query['stack'] = isset($_REQUEST['stack']) ? $_REQUEST['stack'] : array($_SERVER['REQUEST_URI']);
	return tpl_uri($query, $script, $separator);
}

function tpl_uri_call($query = null, $script = null, $stack = null, $separator = '&') {
	$page = isset($_SERVER['REDIRECT_URL']) ? $_SERVER['REDIRECT_URL'] : $_SERVER['SCRIPT_NAME'];
	$query['stack'] = isset($_REQUEST['stack']) ? $_REQUEST['stack'] : array();

	if (null == $script)
		$script = $page;
	if ($script != $page || null != $stack || isset($query['sq']) && 
		(!isset($_GET['sq']) || $query['sq'] != $_GET['sq'])) {
		$query['stack'][] = rtrim(preg_replace('/&?stack(\[|%5B)\d*(\]|%5D)=[^&]*/i', '',
			null != $stack ? $stack : $_SERVER['REQUEST_URI']), '?');
	}
	
	return tpl_uri($query, $script, $separator);
}

function tpl_uri_return($default = '/') {
	if (!empty($_REQUEST['stack'])) {
		$page = end($_REQUEST['stack']);
		if (count($_REQUEST['stack']) > 1)
			$page .= (false === strpos($page,'?') ? '?' : '&').http_build_query(array('stack' => array_slice($_REQUEST['stack'],0,-1)));
		return $page;
	}
	return $default;
}

/**
 * Returns the host serving this request
 * @return	string
 */
function tpl_request_host() {
	$host = isset($_SERVER['HTTP_HOST']) ? $_SERVER['HTTP_HOST'] : $_SERVER['SERVER_NAME'];
	if (false !== ($i = strpos($host, ':')))
		$host = substr($host, 0, $i);
	
	if (isset($_SERVER['HTTPS'])) {
		$uri = 'https://'.$host;
		if (443 != $_SERVER['SERVER_PORT'])
			$uri .= ':'.$_SERVER['SERVER_PORT'];
	}
	else {
		$uri = 'http://'.$host;
		if (80 != $_SERVER['SERVER_PORT'])
			$uri .= ':'.$_SERVER['SERVER_PORT'];
	}
	
	return $uri;
}

/**
 * Implodes array keys into form variable names
 * @param	array	The array to encode keys
 * @param	string	Recursive key prefix [internal only]
 * @return	array
 */
function tpl_form_implode($args, $_prefix = null) {
	$result = array();
	foreach ($args as $key => $value) {
		if (null != $_prefix)
			$key = $_prefix.'['.$key.']';
		if (is_array($value))
			$result = array_merge($result, tpl_form_implode($value, $key));
		else
			$result[$key] = $value;
	}
	return $result;
}

/**
 * Hook for implementing locale modules
 *
 * Use SyndLib::translate() from .inc files to avoid dependancy on 
 * TemplateLib.inc, use the regular tpl_text() from .tpl files.
 *
 * @param	string	String to translate, process, ...
 * @return	string
 */
function tpl_translate() {
	$args = func_get_args();
	return SyndLib::translateArgs($args);
}

/**
 * Hook for implementing locale modules
 *
 * Use SyndLib::translate() from .inc files to avoid dependancy on 
 * TemplateLib.inc, use the regular tpl_text() from .tpl files.
 *
 * @param	string	String to translate, process, ...
 * @return	string
 */
function tpl_text($string) {
	$args = func_get_args();
	return SyndLib::textArgs($args);
}

/**
 * Displays a cached page section
 *
 * Uses the cache if it exists and the system loadlevel is more than
 * the specified load, default to use cache if no load specified.
 *
 * @param	string	Id of section
 * @param	integer	System load to enable caching at
 * @return	bool	TRUE if cache was used
 */
function tpl_cache_display($id, $loadlevel = -1) {
	if (-1 == $loadlevel || SyndLib::loadLevel() >= $loadlevel) {
		if (null != ($content = SyndLib::runHook('cache_get', $id))) {
			print $content;
			return true;
		}
		return false;
	}
	
	global $_tpl_cache_ignore;
	return $_tpl_cache_ignore[$id] = false;
}

/**
 * Deletes a section from cache
 * @param	string	Id of section
 */
function tpl_cache_delete($id) {
	return SyndLib::runHook('cache_del', $id);
}

/**
 * Enter a page section that should be cached
 * @param	string	Id of section
 */
function tpl_cache_enter($id) {
	global $_tpl_cache_ignore;
	if (!isset($_tpl_cache_ignore[$id]))
		ob_start();
}

/**
 * Leaves a page section that should be cached
 * @param	string	Id of section
 */
function tpl_cache_leave($id, $ttl = 1800) {
	if (!isset($GLOBALS['_tpl_cache_ignore'][$id])) {
		$content = ob_get_contents();
		ob_end_clean();
		SyndLib::runHook('cache_set', $id, $content, $ttl);
		print $content;
	}
}

/**
 * Returns md5({file, design}) for this page
 * @param	string	Specific page section
 */
function tpl_cache_pageid($section = null) {
	global $synd_config;
	return SyndLib::hash($_SERVER['REQUEST_URI'], $synd_config['dirs']['design'], $section);
}

/**
 * Tries to find a file for a class.
 *
 * Searches upwards in inheritance tree, returns the absolute path
 * to file. Triggers warning on error such as 'Not found'.
 *
 * @param	string	Class to search for
 * @param	string	File to look for
 * @param	bool	Trigger warning on not found
 * @return	string
 */
function tpl_gui_path($class, $tpl, $warn = true) {
	static $path = array();

	if (!isset($path[$class][$tpl])) {
		$name = $class;

		do {
			if (3 != count($split = explode('_', $name, 3)))
				break;
			$file = tpl_design_path('model/'.$split[1].'/'.$split[2].'/'.$tpl, false);
			if (null != $file) {
				$path[$name][$tpl] = $file;
				break;
			}
		} while (null != ($name = get_parent_class($name)) && !isset($path[$name][$tpl]));

		if (isset($path[$name][$tpl]))
			$path[$class][$tpl] = $path[$name][$tpl];
	}

	if (isset($path[$class][$tpl]))
		return $path[$class][$tpl];

	if (true == $warn)
		trigger_error("No template '$tpl' for class '$class' found", E_USER_WARNING);
	return null;
}

/**
 * Returns the URI to a file.
 * @see		tpl_gui_path()
 */
function tpl_gui_uri($class, $tpl, $warn = true) {
	return substr(tpl_gui_path($class, $tpl, $warn), strlen(rtrim($_SERVER['DOCUMENT_ROOT'],DIRECTORY_SEPARATOR)));

}

/**
 * Renders a class with the specified template.
 * @see		tpl_gui_node()
 * @param	string	Template to render {list_view.tpl, full_view.tpl, ...}
 * @param	array	Temporary data to use in template
 * @return	string
 */
function tpl_gui_class($_class, $_tpl, $_data = null) {
	if (null != $_data)
		extract($_data, EXTR_REFS|EXTR_SKIP);
	ob_start();
	include tpl_gui_path($_class, $_tpl);
	return ob_get_clean();
}

/**
 * Renders an object with the specified template.
 *
 * For example tpl_gui_node($node, 'list_view.tpl') renders $node 
 * using the template model/node/{classId}/list_view.tpl. Returns 
 * the template output.
 *
 * @uses	tpl_gui_path()
 * @param	mixed	Object to render template for or scalar to return immediately
 * @param	string	Template to render {list_view.tpl, full_view.tpl, ...}
 * @param	array	Temporary data to use in template
 * @param	bool	Trigger warning on failure
 * @return	string
 */
function tpl_gui_node($node, $_tpl, $_data = null, $_warn = true) {
	if (!is_object($node))
		return $node;
	if (null == ($_file = tpl_gui_path(get_class($node), $_tpl, $_warn)))
		return null;

	if (null != $_data)
		extract($_data, EXTR_REFS|EXTR_SKIP);
	$_data['node'] = $node;

	ob_start();
	include $_file;
	return ob_get_clean();
}

/**
 * Renders a list of nodes using the specified template
 * @see		tpl_gui_node()
 */
function tpl_gui_list($_list, $_tpl, $_data = null) {
	if (null != $_data)
		extract($_data, EXTR_REFS|EXTR_SKIP);
	ob_start();
	foreach ($_list as $node)
		include tpl_gui_path(get_class($node), $_tpl);
	return ob_get_clean();
}

/**
 * Renders a list of nodes in a table view.
 *
 * Using one template for table head, each row and foot. For example:
 *  thead_view.tpl, trow_view.tpl, tfoot_view.tpl
 *
 * @see		tpl_gui_node()
 * @param	string	Classid to use for rendering thead/tfoot
 * @param	array	List of nodes
 * @param	string	View to use, {thead_, trow_, tfoot_} will be prepended
 * @param	array	Temporary data to use in template
 * @param	string	Page unique id for partial refresh
 * @return	string
 */
function tpl_gui_table($classId, $_list, $_view = 'view.tpl', $_data = null, $_partial = null) {
	$_data['_list'] = $_list;
	$_data['_partial'] = $_partial;

	if (isset($_data['request']['partial']) && $_data['request']['partial'] == $_partial) {
		tpl_gui_class("synd_node_$classId", "thead_$_view", $_data);
		throw new PartialContentException(tpl_gui_list($_list, "trow_$_view", $_data));
	}
	
	$output  = tpl_gui_class("synd_node_$classId", "thead_$_view", $_data);
	$output .= tpl_gui_list($_list, "trow_$_view", $_data);
	$output .= tpl_gui_class("synd_node_$classId", "tfoot_$_view", $_data);
	return $output;
}



/**
 * Resolves a relative design path
 *
 * Tries to find a file by searching the design paths. Returns the 
 * absolute path to file.
 *
 * @param	string	Relative path to file (eg. model/node/text/full_view.tpl)
 * @param	bool	Trigger warning on failure
 * @param	string	Skip this file
 * @return	string
 */
function tpl_design_path($file, $warn = true, $skip = null) {
	global $synd_config;
	$list = $synd_config['dirs']['design'];
	$dir = end($list);

	do {
		if (file_exists($dir.$file) && $dir.$file != $skip)
			return $dir.$file;
	} while (false !== ($dir = prev($list)));

	if ($warn)
		trigger_error("No such file '$file'. Check the template include path (".implode(':', $list).") in synd.inc", E_USER_WARNING);
	return null;
}

/**
 * Returns the path to the file relative to the server's DOCUMENT_ROOT
 * @see		tpl_design_path()
 * @param	string	Relative path to file
 * @return	string
 */
function tpl_design_uri($file, $warn = true) {
	return substr(tpl_design_path($file, $warn), strlen(rtrim($_SERVER['DOCUMENT_ROOT'],DIRECTORY_SEPARATOR)));
}

function tpl_sort_uri($section, $field) {
	$order = isset($_SESSION['synd']['public']['sort'][$section]) ?
		$_SESSION['synd']['public']['sort'][$section] : array();

	$i = 0;
	$desc = false;

	for ($j=0, $cnt=count($order); $i<$cnt && $j<=3; $i++) {
		if (is_string($order[$i])) {
			if ($order[$i] == $field) {
				unset($order[$i]);
				if (empty($order[$i+1]) || 1 == $order[$i+1]) {
					$desc = !empty($order[$i+1]);
					unset($order[($i++)+1]);
				}
			}
			$j++;
		}
	}
	
	array_unshift($order, $desc ? 0 : 1);
	array_unshift($order, $field);

	return tpl_link_call('system','setEnvironment',SyndLib::map('senv', 'sort', $section, array_slice($order,0,$i+2)));
}

function tpl_sort_sql($order, $prepend = null) {
	if (!is_array($order)) {
		if (!isset($_SESSION['synd']['public']['sort'][$order]))
			return null;
		$order = $_SESSION['synd']['public']['sort'][$order];
	}

	$sql = null;
	foreach (_tpl_sort_convert($order) as $column) {
		switch ($column) {
			case SORT_ASC:
				break;
			case SORT_DESC:
				$sql .= ' DESC'; 
				break;
			default:
				if (null != $sql) $sql .= ', ';
				$sql .= $prepend.$column;
		}
	}
	
	return $sql;
}

function tpl_sort_list(&$list, $order, $default = null) {
	if (!is_array($list))
		return;
	if (!is_array($order)) {
		if (isset($_SESSION['synd']['public']['sort'][$order]))
			$order = $_SESSION['synd']['public']['sort'][$order];
		else if (null != $default)
			$order = $default;
		else
			return;
	}

	$list = SyndLib::array_multisort($list, 
		_tpl_sort_convert($order), '_tpl_sort_compare');
}

function tpl_sort_order($section, $prepend = null) {
	if (!isset($_SESSION['synd']['public']['sort'][$section])) 
		return null;
	if (null == $prepend)
		return array_filter($_SESSION['synd']['public']['sort'][$section], '_tpl_sort_filter');
	$order = $_SESSION['synd']['public']['sort'][$section];
	foreach ($order as $i => $column) {
		if (!_tpl_sort_filter($order[$i]))
			unset($order[$i]);
		else if (is_string($column) && !empty($column) && !is_numeric($column))
			$order[$i] = $prepend.$order[$i];
	}
	return $order;
}

/**
 * Switches between two arguments based on the ASC/DESC ordering of a specific field
 * @param	string	The sort section
 * @param	string	The specific field
 * @param	mixed	Argument to return on ASC sorting
 * @param	mixed	Argument to return on DESC sorting
 */
function tpl_sort_switch($section, $field, $a, $b, $default = true) {
	if (empty($_SESSION['synd']['public']['sort'][$section]) || false === ($i = array_search($field, $_SESSION['synd']['public']['sort'][$section])))
		return $default ? $a : $b;
	return !isset($_SESSION['synd']['public']['sort'][$section][$i+1]) || empty($_SESSION['synd']['public']['sort'][$section][$i+1]) ? $b : $a;
}

/**
 * @access	private
 */
function _tpl_sort_filter($value) {
	return (is_scalar($value) || is_null($value)) && preg_match('/^\w*$/', $value);
}

/**
 * @access	private
 */
function _tpl_sort_convert($order) {
	foreach ($order	as $key => $value) {
		if (is_numeric($value) || empty($value))
			$order[$key] = $value ? SORT_ASC : SORT_DESC;
	}
	return $order;
}

/**
 * @access	private
 */
function _tpl_sort_compare(&$a, &$b, $key) {
	return strnatcasecmp(
		isset($a->data[$key]) ? $a->data[$key] : null, 
		isset($b->data[$key]) ? $b->data[$key] : null);
}

function tpl_form_input($name, $value = null, $attrib = null) {
	$result = '<input type="text" name="'.$name.'" id="'.$name.'"';
	if (null != $value)
		$result .= ' value="'.tpl_attribute($value).'"';
	if (null != $attrib)
		$result .= ' '.tpl_html_implode($attrib);
	return $result.' />';
}

function tpl_form_textarea($id, $content, $attrib = null, $max = 20, $min = 4, $default = 4) {
	if (!isset($attrib['id']))
		$attrib['id'] = $id;
	if (!isset($attrib['cols']))
		$attrib['cols'] = '80';
	if (!isset($attrib['rows'])) {
		$rows = substr_count(trim($content), "\n");
		preg_match_all("/[^\n]\{{$attrib['cols']},}(?:\n|$)/s", $content, $matches);
		foreach ($matches[0] as $string) 
			$rows += round(strlen($string)/$attrib['cols']);
		if (0 == $rows)
			$attrib['rows'] = $default;
		else if ($rows <= $min)
			$attrib['rows'] = $min;
		else if ($rows < $max)
			$attrib['rows'] = $rows+2;
		else
			$attrib['rows'] = $max;
	}

	$default = '<textarea name="'.$id.'"';
	if (null != $attrib)
		$default .= ' '.tpl_html_implode($attrib);
	$default .= '>'.htmlspecialchars($content).'</textarea>';
	
	return $default;
}

function tpl_form_htmlarea($id, $content, $style = null, $attrib = null) {
	if (!isset($style['width']) && !isset($attrib['cols']))
		$attrib['cols'] = '80';
	if (!isset($style['height']) && !isset($attrib['rows']))
		$attrib['rows'] = '15';

	$default = '<textarea name="'.$id.'"';
	if (null != $style)
		$default .= ' style="'.tpl_html_implode($style,':','; ').'"';
	if (null != $attrib)
		$default .= ' '.tpl_html_implode($attrib);
	$default .= '>'.htmlspecialchars($content).'</textarea>';

	if (null != ($result = Module::runHook('htmlarea', $id, $content, $style, $attrib, $default)))
		return $result;
	return $default;
}

/**
 * Assembles the options for a select box.
 * @param	array	array(key=>value) gives <option value="key">value</option>
 * @param	mixed	The selected key
 * @param	array	Element attributes
 * @return	string
 */
function tpl_form_options($options, $selected = null, $attrib = null) {
	ob_start();
	
	foreach ($options as $key => $value) {
		$style = null;
		if (is_array($value)) {
			$style = ' style="'.$value[1].'"';
			$value = $value[0];
		}
	
		if ($key === $selected || 
			!is_scalar($selected) && isset($selected[$key]) ||
			 is_scalar($selected) && null !== $selected && '' !== $selected && (string)$key == (string)$selected) {
			print "<option{$style} selected=\"selected\" ";
		}
		else {
			print "<option{$style} ";
		}
		
		print "value=\"$key\"";

		if (null != $attrib) {
			print ' ';
			print tpl_html_implode($attrib);
		}
		
		print ">";
		print $value;
		print "</option>\n";
	}

	return ob_get_clean();
}

/**
 * Creates a html checkbox
 * @param	string	Name of checkbox
 * @param	bool	Checked or not
 * @param	string	Value of checkbox
 * @param	string	Id of checkbox, defaults to same as name
 * @param	array	Element attributes
 * @return	string
 */
function tpl_form_checkbox($name, $checked = false, $value = 1, $id = null, $attrib = null) {
	$result = 
		'<input type="checkbox" name="'.$name.'" value="'.tpl_attribute($value).
		'" id="'.tpl_attribute(null!=$id?$id:$name).'"';
	if (null != $attrib)
		$result .= ' '.tpl_html_implode($attrib);
	if (true == $checked)
		$result .= ' checked="checked"';
	return $result.' />';
}

/**
 * Returns a html radiobutton contruct
 * @param	string	Name of radiobutton
 * @param	string	Value of selected button. If same as value this will be 'checked'
 * @param	string	Value of radiobutton
 * @param	string	Id of radiobutton, defaults to name[value]
 * @param	array	Element attributes
 * @return	string
 */
function tpl_form_radiobutton($name, $selected, $value = 1, $id = null, $attrib = null) {
	$result = 
		'<input type="radio" name="'.$name.'" value="'.tpl_attribute($value).
		'" id="'.tpl_attribute(null!=$id?$id:$name."[$value]").'"';
	if (null != $attrib)
		$result .= ' '.tpl_html_implode($attrib);
	if ($value == $selected)
		$result .= ' checked="checked"';
	return $result.' />';
}

/**
 * Returns a html input contruct
 * @param	string	Name of element
 * @param	string	Value of element
 * @param	array	Element attributes
 * @return	string
 */
function tpl_form_text($name, $value = '', $id = null, $attrib = null) {
	$result = 
		'<input type="text" name="'.$name.'" value="'.tpl_attribute($value).
		'" id="'.tpl_attribute(null!=$id?$id:$name).'"';
	if (null != $attrib)
		$result .= ' '.tpl_html_implode($attrib);
	return $result.' />';
}

function tpl_html_implode($vals, $pre = '="', $post = '" ') {
	if (!empty($vals)) {
		$result = '';
		foreach ($vals as $key => $value)
			$result .= $key.$pre.$value.$post;
		return rtrim($result);
	}
}

/**
 * Activates urls in a text.
 *
 * www.example.com becomes <a href="http://www.example.com">www.example.com</a>
 *
 * @param	string	Text to process
 * @param	string	Link target {_self, _new, ..}
 * @return	string
 */
function tpl_html_links($text, $target = '_self') {
	return preg_replace_callback(
		'/(<(a)?)?(?(1).*?(?(2)<\/a>|>)|((?:([a-z]+:\/\/)|(?<!\/|>)(www|ftp)\.)[\w\.]+[\/\w.,~%?&=_+\-\[\]#:;@]*?|([\w\.+-]+@[\w\-\.]+))(?=$|[.,]?[\s>"]|&gt;|&quot;))/i', 
		array(new TplHtmlLinkActivator($target), 'replace'), $text);
}

/**
 * @private
 */
class TplHtmlLinkActivator {
	private $_target = null;
	
	function __construct($target) {
		$this->_target = $target;
	}
	
	function replace($m) {
		if (!empty($m[6]))
			return "<a href=\"mailto:{$m[6]}\">{$m[6]}</a>";
		if (!empty($m[3])) {
			$uri = "<a target=\"{$this->_target}\" href=\"";

			if ('' == $m[4]) {
				switch (strtolower($m[5])) {
					case 'www':
						$uri .= 'http://'; break;
					case 'ftp':
						$uri .= 'ftp://'; break;
					default:
						$uri .= '//';
				}
			}

			$uri .= "{$m[3]}\">{$m[3]}</a>";
			return $uri;
		}
		return $m[0];
	}
}

/**
 * Activates links and formats the text
 *
 * Urls are activate via tpl_html_links(), newlines are replaced with
 * <br /> and spaces and tabs at the beginning of lines transformed into
 * non breaking spaces &nbsp;
 *
 * @param	string	Text to transform
 * @return	string
 */
function tpl_html_format($text) {
	return preg_replace(
		array(
			"/(\r?\n){2,}/",
			"/(\r?\n)/",
			"/\t/",
			"/  /",
			),
		array(
			"<br /><br />",
			"<br />",
			"&nbsp;&nbsp;&nbsp;&nbsp;",
			"&nbsp;&nbsp;",
			),
		tpl_html_links(htmlspecialchars(trim($text))));
}

/**
 * Formats the text
 * @param	string	Text to transform
 * @return	string
 */
function tpl_text_format($text) {
	return preg_replace(
		array(
			"/(\r?\n){2,}/",
			),
		array(
			"\r\n\r\n",
			),
		trim($text));
}

/**
 * Remove HTML tags, including invisible text such as style and
 * script code, and embedded objects.  Add line breaks around
 * block-level tags to prevent word joining after tag removal.
 */
function tpl_strip_html_tags( $text ) {
    $text = preg_replace(
        array(
          // Remove invisible content
            '@<head[^>]*?>.*?</head>@siU',
            '@<style[^>]*?>.*?</style>@siU',
            '@<script[^>]*?.*?</script>@siU',
            '@<object[^>]*?.*?</object>@siU',
            '@<embed[^>]*?.*?</embed>@siU',
            '@<applet[^>]*?.*?</applet>@siU',
            '@<noframes[^>]*?.*?</noframes>@siU',
            '@<noscript[^>]*?.*?</noscript>@siU',
            '@<noembed[^>]*?.*?</noembed>@siU',
          // Add line breaks before and after blocks
            '@</?(address|blockquote|center|del)@iU',
            '@</?(div|(h[1-9])|ins|isindex|p|pre)@iU',
            '@</?(dir|dl|dt|dd|li|menu|ol|ul)@iU',
            '@</?(table|th|td|caption)@iU',
            '@</?(form|button|fieldset|legend|input)@iU',
            '@</?(label|select|optgroup|option|textarea)@iU',
            '@</?(frameset|frame|iframe)@iU',
            '@<br[ \t/>]@iU',
        ),
        array(
            ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
            "\n\$0", "\n\$0", "\n\$0", "\n\$0", "\n\$0", "\n\$0",
            "\n\$0", "\n\$0",
        ),
        $text );
    return strip_tags( $text );
}

function tpl_load_script($uri, $defer = true) {
	global $_tpl_load_header;
	if (!isset($_tpl_load_header)) {
		SyndLib::attachHook('html_head', '_tpl_load_callback');
		$_tpl_load_header = array();
	}
	$_tpl_load_header[$uri] = "<script type=\"text/javascript\" src=\"$uri\"".($defer?" defer=\"defer\"":"")."></script>";
}

function tpl_load_stylesheet($uri, $media = 'all') {
	global $_tpl_load_header;
	if (!isset($_tpl_load_header)) {
		SyndLib::attachHook('html_head', '_tpl_load_callback');
		$_tpl_load_header = array();
	}
	$_tpl_load_header[$uri] = "<link rel=\"stylesheet\" href=\"$uri\" type=\"text/css\" media=\"$media\" />";
}

/**
 * @access	private
 */
function _tpl_load_callback(&$result) {
	global $_tpl_load_header;
	$result .= "\t".implode("\n\t", $_tpl_load_header)."\n";
}

/**
 * Defaults a number of values, returns &nbsp; if none != null
 * @param	mixed	$v1,...	First one != null will be returned
 * @return	mixed
 */
function tpl_def() {
	for ($i=0, $c=func_num_args(); $i<$c; $i++) {
		if (null != ($arg = func_get_arg($i)))
			return $arg;
	}
	return '&nbsp;';
}

/**
 * Defaults a number of values
 * @param	mixed	$v1,...	First one != null will be returned
 * @return	mixed
 */
function tpl_default() {
	for ($i=0, $c=func_num_args(); $i<$c; $i++) {
		if (null != ($arg = func_get_arg($i)))
			return $arg;
	}
	return '';
}

/**
 * HTML escapes the " character in values, default to null
 * @see		tpl_def()
 * @param	mixed	$v1,...	First one != null will be returned
 * @return	mixed
 */
function tpl_value() {
	for ($i=0, $c=func_num_args(); $i<$c; $i++) {
		if (null != ($arg = func_get_arg($i)))
			return is_scalar($arg) ? str_replace('"', '&quot;', $arg) : $arg;
	}
	return null;
}

/**
 * Html quotes the first non-empty parameter
 * @see		htmlspecialchars()
 * @param	string,...
 * @return	string
 */
function tpl_quote() {
	for ($i=0, $c=func_num_args(); $i<$c; $i++) {
		if (null != ($arg = func_get_arg($i)))
			return htmlspecialchars($arg);
	}
	return null;
}

/**
 * Html quotes the first non-empty parameter and removes newlines
 * @see		htmlspecialchars()
 * @param	string,...
 * @return	string
 */
function tpl_attribute() {
	for ($i=0, $c=func_num_args(); $i<$c; $i++) {
		if (null !== ($arg = func_get_arg($i)) || '' !== $arg)
			return htmlspecialchars(strtr($arg, "\r\n", "  "));
	}
	return null;
}

/**
 * Maps a callback onto a number of parameters
 * @param	callback	If null, the first additional parameter is returned
 * @return	mixed
 */
function tpl_filter($callback) {
	if (null == $callback) 
		return func_get_arg(1);
	$args = func_get_args();
	array_shift($args);
	return call_user_func_array($callback, $args);
}

function tpl_date($format) {
	$args = func_get_args();
	array_shift($args);
	for ($i=0; $i<count($args); $i++) {
		if (null != $args[$i])
			return is_numeric($args[$i]) ? date($format, $args[$i]) : $args[$i];
	}
	return null;
}

/**
 * Presents a date adjusted to the clients timezone.
 *
 * Uses client side javascript to detect the clients timezone offset
 * and adjusts dates accordingly.
 *
 * <pre>
 * Specifiers (addional to the standard strftime() ones):
 *   %O		Difference to Greenwich time (GMT) in hours		Example: +0200
 * </pre>
 *
 * @see		strftime()
 * @param	string	Format string
 * @param	integer	Timestamp
 * @return	string
 */
function tpl_strftime($format, $time = null) {
	if (null === $time) 
		$time = time();
	else if ($time <= 0)
		return '';
	$offset = /*isset($_SESSION['synd']['timezone_offset']) ? 
		$_SESSION['synd']['timezone_offset'] : */0;
	$gmt = sprintf('%+03d%02d', -floor($offset/60), abs($offset%60));
	$format = str_replace('%O', $gmt, $format);
	$format = str_replace('%Z', "GMT$gmt", $format);
	//return gmstrftime($format, $time - $offset*60);
	return strftime($format, $time - $offset*60);
}

/**
 * Formats a durations into hours and minutes
 * @param	integer	Duration in seconds
 * @param	string	Name of hours 
 * @param	string	Name of minutes
 * @param	string	Name of seconds
 * @param	string	Name of days
 * @return	string
 */
function tpl_duration($duration, $d = null, $h = ' Hours', $m = ' Minutes', $s = ' Seconds') {
	$str = $duration < 0 ? '-' : '';
	if (null != $d && $days = floor(abs($duration)/3600/24)) {
		$str .= SyndLib::translate("%s$d", $days);
		$duration -= $days*3600*24;
	}
	if ($hours = floor(abs($duration)/3600))
		$str .= SyndLib::translate("%s$h", $hours);
	if ($minutes = floor(abs($duration)%3600/60))
		$str .= ' '.SyndLib::translate("%s$m", $minutes);
	if ((null == $str || '-' == $str) && $duration)
		$str .= SyndLib::translate("%s$s", round(abs($duration)));
	return trim($str);
}

/**
 * Cuts a string if longer that $maxlen and appends $append
 * @param	string	String to chop
 * @param	integer	Max length of string
 * @param	string	String to append if chopped
 * @return	string
 */
function tpl_chop($str, $maxlen, $append = '..') {
	if ($maxlen && strlen($str) > $maxlen)
		return substr($str,0,$maxlen-strlen($append)).$append;
	return $str;
}

/**
 * Iterates over a list of values
 *
 * <code>
 *  Calling it like
 *   <tr class="<?= tpl_cycle(array('odd','even')) ?>">
 *  will produce
 *   <tr class="even">
 *   <tr class="odd">
 *   <tr class="even">
 *   ...
 * </code>
 *
 * @param	array	Values to cycle, defaults to last used
 * @return	mixed
 */
function tpl_cycle($values = null) {
	static $v = null, $i = 0;
	if (null !== $values)
		$v = $values;
	else
		$values = $v;
	if (!isset($v[$i]))
		$i = 0;
	return $v[$i++];
}

class EmailReplaceFunctor {
	var $_subject;
	
	function __construct($subject) {
		$this->_subject = $subject;
	}
	
	function replace($m) {
		global $synd_user;
		if ($synd_user->isNull())
			return strtr($m[0], array('@'=>' AT ', '.'=>' DOT '));
		
		if (null != $this->_subject) {
		    return '<a href="mailto:'.tpl_attribute($m[0]).'?subject='.tpl_attribute($this->_subject).'">'.$m[0].'</a>';
		}
		
		return '<a href="mailto:'.tpl_attribute($m[0]).'">'.$m[0].'</a>';
	}
}

/**
 * Formats a mailto: link
 * 
 * If no user is logged in the email will be obfuscated using the
 * 'example AT example DOT com' style. Will use htmlspecialchars()
 * to escape any html chars
 *
 * @see		htmlspecialchars()
 * @param	string	Email address
 * @return	string
 */
function tpl_email($email, $subject = null) {
	return preg_replace_callback(
		'/[\w\-\+\._]+@[\w\-\.]+/', 
		array(new EmailReplaceFunctor($subject), 'replace'), 
		htmlspecialchars($email));
}
