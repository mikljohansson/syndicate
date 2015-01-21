<?php
/**
 * @category	Testing
 * @package		PHPUnit2
 * @version		CVS: $Id: TestRunner.php,v 0.0 2006/01/01 00:00:00 mikl Exp $
 * @link		http://pear.php.net/package/PHPUnit2
 * @since		File available since Release 2.x.x
 */

require_once 'PHPUnit2/Framework.php';
require_once 'PHPUnit2/Runner/BaseTestRunner.php';
require_once dirname(__FILE__).'/TestResult.php';

/**
 * @package		PHPUnit2
 */
class PHPUnit2_HtmlUI_TestRunner {
	/**
	 * Service an HTTP request using global variables $_REQUEST and $_SERVER
	 * @throws	RuntimeException
	 */
	public static function main() {
		if (!isset($_REQUEST, $_SERVER['SCRIPT_FILENAME']))
			throw new RuntimeException('$_REQUEST and $_SERVER[\'SCRIPT_FILENAME\'] variables must be set');
		$runner = new PHPUnit2_HtmlUI_TestRunner();
		$runner->start(array(dirname($_SERVER['SCRIPT_FILENAME'])), $_SERVER, $_REQUEST);
	}
	
	/**
	 * Service an HTTP request
	 * @throws	RuntimeException
	 * @param	array	Top level directories containing unit tests
	 * @param	array	Server variables ($_SERVER)
	 * @param	array	Form variables ($_REQUEST)
	 */
	function start(Array $dirs, Array $server, Array $request) {
		$classes = get_declared_classes();
		$sq = isset($request['sq']) ? $request['sq'] : '';

		foreach ($dirs as $root) {
			if (false === ($dir = realpath(rtrim($root, DIRECTORY_SEPARATOR).DIRECTORY_SEPARATOR.$sq)))
				continue;
			if (0 !== strpos($dir, $root))
				throw new ForbiddenException();
			$this->scanDirectory(new RecursiveDirectoryIterator($dir), !empty($request['recurse']));
		}

		$toggle = true;
		$suites = array();
		
		$classes = array_diff(get_declared_classes(), $classes);
		sort($classes);

		foreach ($classes as $class) {
			$reflector = new ReflectionClass($class);
			if (!$reflector->isInstantiable())
				continue;
			else if (is_subclass_of($class, 'PHPUnit2_Framework_TestCase'))
				$suite = new PHPUnit2_Framework_TestSuite($reflector);
			else if (is_subclass_of($class, 'PHPUnit2_Framework_TestSuite'))
				$suite = new $class();
			else
				continue; 
			
			$aggregator = new PHPUnit2_HtmlUI_TestResult();
			$result = new PHPUnit2_Framework_TestResult();
			$result->addListener($aggregator);

			$run = !empty($request['suites']) && in_array($class, $request['suites']);
			$toggle = $toggle && $run;
			
			$timestamp = null;
			if ($run) {
				$timestamp = microtime(true);
				$suite->run($result);
				$timestamp = microtime(true) - $timestamp;
			}
			
			$suites[] = array(
				'suite' 	=> $suite, 
				'result'	=> $result, 
				'tests'		=> $aggregator, 
				'wasrun'	=> $run, 
				'duration'	=> $timestamp, 
				'reflector'	=> $reflector);
		}

		usort($dirs, array($this, '_compare'));
		$options = array(
			'recurse' => !empty($request['recurse']),
			'showok' => !empty($request['showok']),
			);
		$uri = parse_url($server['REQUEST_URI']);
		
		$this->display('assets/index.tpl', array(
			'sq'		=> $sq, 
			'dirs'		=> $dirs, 
			'suites'	=> $suites, 
			'toggle'	=> $toggle,
			'options'	=> $options,
			'uri'		=> $server['REQUEST_URI'],
			'uripath'	=> $uri['path'],
			));
	}
	
	function _compare($a, $b) {
		return strcasecmp(basename($a), basename($b));
	}
	
	/**
	 * Render a template to STDOUT
	 * @throws	RuntimeException
	 * @param	string	Path to template
	 * @param	array	Template parameters
	 */
    function display($_tpl, $_data = null) {
		if (null != $_data)
			extract($_data, EXTR_SKIP);
		include $this->getAssetPath($_tpl);
	}
	
	/**
	 * Resolves the absolute path of an asset
	 * @param	string	Relative path to asset
	 * @return	string
	 */
	protected function getAssetPath($asset) {
		$path = dirname(__FILE__).DIRECTORY_SEPARATOR.str_replace('/', DIRECTORY_SEPARATOR, $asset);
		if (file_exists($path))
			return $path;
		throw new RuntimeException("Could not resolve absolute path to asset '$asset'");
	}
	
	/**
	 * Scan a directory for php files and include them
	 * @param	DirectoryIterator	Iterator to walk
	 * @param	bool				Recurse down in folder tree
	 */
	protected function scanDirectory(RecursiveDirectoryIterator $iterator, $recurse) {
		for ($iterator->rewind(); $iterator->valid(); $iterator->next()) {
			if ($iterator->isDir()) {
				if ('.' != substr($iterator->getFilename(),0,1) && '_' != substr($iterator->getFilename(),0,1) && $recurse)
					$this->scanDirectory($iterator->getChildren(), $recurse);
			}
			else if ('php' == strtolower(SyndLib::fileExtension($iterator->getPathname())))
				include_once $iterator->getPathname();
		}
	}
}
