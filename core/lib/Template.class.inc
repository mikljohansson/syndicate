<?php
require_once 'core/lib/SyndLib.class.inc';

/**
 * Template class.
 *
 * Templates is written in html/php, assigned varibles will be
 * extracted into the current symbolspace and the template file 
 * included. The output will be returned or displayed.
 *
 * @access		public
 * @package		synd.core.lib
 */
class Template {
	protected $_layouts = null;
	protected $_rewrite = true;
	protected $_request = null;
	protected $_data = null;
	
	private $_cache = null;
	private $_cycle = null;
	private $_sequence = 1;
	private $_included = null;
	
	/**
	 * @param	array	The layout directories to search for templates (must end with DIRECTORY_SEPARATOR)
	 * @param	string	URL base, for example '/synd/' when using mod_rewrite or '/synd/index.php?sq=' when not
	 */
	function __construct($layouts = null, $request = null) {
		$this->_layouts = $layouts;
		if (null !== $request) {
			$this->_request = $request;
			$this->_rewrite = false === strpos($request->uribase, '?');
		}
	}

	/**
     * Renders and prints the output from a template
     *
     * Extracts the assigned data and any supplied temporary data
     * into the current symbolspace and includes the specified file. 
	 *
     * @throws	InvalidArgumentException
     * @param	string	Relative or absolute path to template
     * @param	array	Local variables, used only for this template
	 */
    function display($_tpl, $_data = null) {
    	if (null != $_data)
    		extract($_data, EXTR_SKIP);
    	if (null != $this->_data)
			extract($this->_data, EXTR_SKIP);

		if (file_exists($_tpl))
			return include($_tpl);
		foreach ((array)$this->_layouts as $_layout) {
			if (file_exists($_layout.$_tpl))
				return include($_layout.$_tpl);
		}
		
		throw new InvalidArgumentException("Template '$_tpl' not found");
    }

    /**
     * Renders a template and returns the output
     * @throws	InvalidArgumentException
     * @param	string	Relative or absolute path to template
     * @param	array	Local variables, used only for this call
     * @return	string
     */
    function fetch($_tpl, $_data = null) {
		ob_start();
		$this->display($_tpl, $_data);
		return ob_get_clean();
    }
    
	/**
     * Renders a template once
     * @throws	InvalidArgumentException
     * @param	string	Relative or absolute path to template
     * @param	array	Local variables, used only for this template
	 */
    function displayonce($_tpl, $_data = null) {
		if (!in_array($_tpl, (array)$this->_included)) {
			$this->_included[] = $_tpl;
			$this->display($_tpl, $_data);
		}
	}    

    /**
     * Resolve an object template
     * @throws	InvalidArgumentException
     * @param	object	The object to resolve template for
     * @param	string	Relative path to template
     * @param	bool	Throw exception on invalid template
     * @return	bool
     */
	function path($class, $template, $throw = true) {
		do {
			if (isset($this->_cache[$class][$template]))
				return $this->_cache[$class][$template];
			if (false !== ($i = strpos($class, '_')) && false !== ($j = strpos($class, '_', $i+1))) {
				foreach ((array)$this->_layouts as $layout) {
					if (file_exists($file = $layout.'model'.DIRECTORY_SEPARATOR.substr($class, $i+1, $j-$i-1).DIRECTORY_SEPARATOR.substr($class, $j+1).DIRECTORY_SEPARATOR.$template))
						return $this->_cache[$class][$template] = $file;
				}
			}
		} while (null != ($class = get_parent_class($class)));
		if ($throw)
			throw new InvalidArgumentException("Template '$template' not found for class '".func_get_arg(0)."'. Check the template include path (".implode(':', (array)$this->_layouts).") in synd.inc");
		return false;
    }
    
    /**
     * Renders an object
     *
     * <code>
     *  class synd_node_generic {}
     *  class synd_node_specific extends synd_node_generic {}
     *  
     *  $page = new Template(array('/var/www/templates/blueskin/', '/var/www/templates/default/'));
     *  $page->render(new synd_node_specific(), 'title.tpl');
     * </code>
     *
     * The example would attempt to find the template as
     *
     *  1) /var/www/templates/blueskin/model/node/specific/title.tpl
     *  2) /var/www/templates/default/model/node/specific/title.tpl
     *  3) /var/www/templates/blueskin/model/node/generic/title.tpl
     *  4) /var/www/templates/default/model/node/generic/title.tpl
     *
     * @throws	InvalidArgumentException
     * @param	object	The object to render, available as $node in the template
     * @param	string	Relative path to template
     * @param	array	Local variables, used only for this call
     * @param	bool	Throw exception on invalid template
     */
    function render($node, $_tpl, $_data = null, $_throw = true) {
        if (null !== $node && false != ($_file = $this->path(get_class($node), $_tpl, $_throw))) {
			if (null != $this->_data)
				extract($this->_data);
			if (null != $_data)
				extract($_data);
        	include $_file;
        }
    }

    /**
     * Renders an object and returns the output
     * @throws	InvalidArgumentException
     * @param	object	The object to render, available as $node in the template
     * @param	string	Relative path to template
     * @param	array	Local variables, used only for this call
     * @param	bool	Throw exception on invalid template
     */
    function fetchnode($node, $_tpl, $_data = null, $_throw = true) {
		ob_start();
		$this->render($node, $_tpl, $_data, $_throw);
		return ob_get_clean();
    }

    /**
     * Iterate over and render a list of objects
     * @throws	InvalidArgumentException
     * @param	Traversable		The objects to render, available as $node in the template
     * @param	string			Relative path to template
     * @param	array			Local variables, used only for this call
     */
    function iterate($_list, $_tpl, $_data = null) {
		if (null != $this->_data)
			extract($this->_data);
		if (null != $_data)
			extract($_data);
        foreach ($_list as $node)
			include $this->path(get_class($node), $_tpl, true);
    }

    /**
     * Render a list of objects and returns the output
     * @throws	InvalidArgumentException
     * @param	Traversable		The objects to render, available as $node in the template
     * @param	string			Relative path to template
     * @param	array			Local variables, used only for this call
     */
    function fetchiterator($_list, $_tpl, $_data = null) {
		ob_start();
		$this->iterate($_list, $_tpl, $_data);
		return ob_get_clean();
    }

    /**
     * Assign a value to the specified varible
     * @param	string	Name of varible in template
     * @param	mixed	Value to assign
     */
    function assign($key, $value) {
   		$this->_data[$key] = $value;
    }

	/**
	 * Appends the value to the end of an existing array or string.
	 * @see		Template::assign()
	 */
    function append($key, $value) {
		if (is_array($value)) {
			if (!isset($this->_data[$key]))
				$this->_data[$key] = array();
			$this->_data[$key][] = $value;
		}
		else {
			if (!isset($this->_data[$key]))
				$this->_data[$key] = null;
   			$this->_data[$key] .= $value;
   		}
    }

	/**
	 * HTML escapes a value
	 * @param	string	The value to quote
	 * @return	string
	 */
	function quote($value) {
		return htmlspecialchars($value);
	}
	
	/**
	 * Text localization hook
	 * @param	string		String to localize
	 * @param	mixed,...	Extra parameters to sprintf into string
	 * @return	string
	 */
	function translate() {
		$args = func_get_args();
		return SyndLib::translateArgs($args);
	}

	/**
	 * Text localization hook with parameter escaping
	 * @param	string		String to localize
	 * @param	mixed,...	Extra parameters to sprintf into string
	 * @return	string
	 */
	function text() {
		$args = func_get_args();
		return SyndLib::textArgs($args);
	}

	/**
	 * Iterates over a list of values
	 *
	 * <code>
	 *  Calling it like
	 *
	 *   <tr class="<?= $this->cycle(array('odd','even')) ?>">
	 *   <tr class="<?= $this->cycle() ?>">
	 *   <tr class="<?= $this->cycle() ?>">

	 *  will produce
     *
	 *   <tr class="even">
	 *   <tr class="odd">
	 *   <tr class="even">
	 *   ...
	 * </code>
	 *
	 * @param	array	Values to cycle, defaults to last used
	 * @return	mixed
	 */
	function cycle($values = null) {
		if (null !== $values)
			$this->_cycle = $values;
		else if (false === next($this->_cycle))
			reset($this->_cycle);
		return current($this->_cycle);
	}
	
	/**
	 * Generates an incrementing sequence of values
	 * @param	int		Value to set
	 * @return	int
	 */
	function sequence($value = null) {
		if (null !== $value)
			$this->_sequence = $value;
		return $this->_sequence++;
	}

	/**
	 * Activates urls in a text.
	 *
	 * www.example.com becomes <a href="http://www.example.com">www.example.com</a>
	 *
	 * @param	string	Text to process
	 * @return	string
	 */
	function links($text) {
		return preg_replace_callback(
			'/(<(a)?)?(?(1).*?(?(2)<\/a>|>)|((?:([a-z]+:\/\/)|(?<!\/|>)(www|ftp)\.)[\w\.]+[\/\w.,~%?&=_+\-\[\]#:;@]*?|([\w\.+-]+@[\w\-\.]+))(?=$|[.,]?[\s>"]|&gt;|&quot;))/i', 
			array($this, '_callback_replace_links'), $text);
	}

	/**
	 * @access	private
	 */
	function _callback_replace_links($m) {
		if (!empty($m[6]))
			return "<a href=\"mailto:{$m[6]}\">{$m[6]}</a>";
		if (!empty($m[3])) {
			$uri = "<a href=\"";

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
	function format($text) {
		return str_replace(
			array(
				"\r\n",
				"\n",
				"\t",
				"  ",
				"\n ",
				),
			array(
				"\n",
				"<br />\r\n",
				"&nbsp;&nbsp;&nbsp;&nbsp;",
				"&nbsp;&nbsp;",
				"\n&nbsp;",
				),
			$this->links(htmlspecialchars($text)));
	}
	
	protected function _rewrite($args, $stack = null, $separator = '&') {
		$path = $this->_request->uribase;
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
			$path .= $this->_rewrite ? '?' : $separator;
			$path .= http_build_query($query, false, $separator);
		}

		return $path;
	}

	function uri() {
		return $this->_rewrite(func_get_args());
	}
	
	function jumpuri() {
		return $this->_rewrite(func_get_args(), isset($this->_request['stack']) ? $this->_request['stack'] : null);
	}

	function href() {
		return $this->_rewrite(func_get_args(), null, '&amp;');
	}

	function call() {
		$stack = isset($this->_request['stack']) ? (array)$this->_request['stack'] : array();
		$uri = $this->_request->getVirtualPath();
		
		if (!empty($this->_request->get)) {
			$uri .= $this->_rewrite ? '?' : '&amp;';
			$uri .= http_build_query($this->_request->get, false, '&amp;');
		}
		
		$stack[] = $uri;
		return $this->_rewrite(func_get_args(), $stack, '&amp;');
	}

	function calluri() {
		$stack = isset($this->_request['stack']) ? (array)$this->_request['stack'] : array();
		$uri = $this->_request->getVirtualPath();
		
		if (!empty($this->_request->get)) {
			$uri .= $this->_rewrite ? '?' : '&amp;';
			$uri .= http_build_query($this->_request->get, false, '&amp;');
		}
		
		$stack[] = $uri;
		return $this->_rewrite(func_get_args(), $stack);
	}

	function jump() {
		return $this->_rewrite(func_get_args(), isset($this->_request['stack']) ? $this->_request['stack'] : null, '&amp;');
	}

	/**
	 * Builds an url that duplicates a request
	 * @param	Request	Request to merge args into
	 * @param	array	Additional args to merge
	 * @return	string
	 */
	function merge($args) {
		return $this->_rewrite($this->_request->context, null, '&amp;').'?'.http_build_query(array_merge($this->_request->get, $args));
	}
}

/**
 * Template with a default file to render
 */
class DefaultTemplate extends Template {
	private $_template = null;
	
	/**
	 * @param	string	Default template
	 */
	function __construct($template, $layouts = null) {
		parent::__construct($layouts);
		$this->_template = $template;
	}
	
	static function factory($template, Template $parent) {
		$instance = new self($template, $parent->_layouts);
		$instance->_data = $parent->_data;
		return $instance;
	}
	
	/**
	 * Renders the default template
	 * @return	string
	 */
	function __toString() {
		try {
			return $this->fetch($this->_template);
		}
		catch (Exception $e) {
			trigger_error($e->getMessage(), E_USER_ERROR);
		}
	}
}
