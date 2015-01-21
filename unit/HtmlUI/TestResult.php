<?php
/**
 * @category	Testing
 * @package		PHPUnit2
 * @version		CVS: $Id: TestRunner.php,v 0.0 2006/01/01 00:00:00 mikl Exp $
 * @link		http://pear.php.net/package/PHPUnit2
 * @since		File available since Release 2.x.x
 */

require_once 'PHPUnit2/Framework.php';
require_once 'PHPUnit2/Util/Printer.php';

/**
 * @package		PHPUnit2
 */
class PHPUnit2_HtmlUI_TestResult implements PHPUnit2_Framework_TestListener, IteratorAggregate {
    const FAILURE = 0;
    const SUCCESS = 1;
    const SKIPPED = 2;
    
    protected $_result = null;
    protected $_exception = null;
    protected $_timestamp = null;
    protected $_results = array();
    
    public function addError(PHPUnit2_Framework_Test $test, Exception $e) {
    	$this->_result = self::FAILURE;
    	$this->_exception = $e;
    }

    public function addFailure(PHPUnit2_Framework_Test $test, PHPUnit2_Framework_AssertionFailedError $e) {
    	$this->_result = self::FAILURE;
    	$this->_exception = $e;
    }

    public function addIncompleteTest(PHPUnit2_Framework_Test $test, Exception $e) {
    	$this->_result = self::FAILURE;
    	$this->_exception = $e;
    }

    public function addSkippedTest(PHPUnit2_Framework_Test $test, Exception $e) {
    	$this->_result = self::SKIPPED;
    }

    public function startTestSuite(PHPUnit2_Framework_TestSuite $suite) {}

    public function endTestSuite(PHPUnit2_Framework_TestSuite $suite) {}

    public function startTest(PHPUnit2_Framework_Test $test) {
    	$this->_result = self::SUCCESS;
    	$this->_exception = null;
    	$this->_timestamp = microtime(true);
    }

    public function endTest(PHPUnit2_Framework_Test $test) {
    	$end = microtime(true);
    	$this->_results[$test->getName()] = new PHPUnit2_HtmlUI_Result($test, $this->_result, $end - $this->_timestamp, $this->_exception);
    }
    
    public function getIterator() {
    	return new ArrayIterator($this->_results);
    }
}

class PHPUnit2_HtmlUI_Result {
	protected $_test = null;
	protected $_code = null;
	protected $_duration = null;
	protected $_exception = null;
	
	public function __construct(PHPUnit2_Framework_Test $test, $code, $duration, $exception) {
		$this->_test = $test;
		$this->_code = $code;
		$this->_duration = $duration;
		$this->_exception = $exception;
	}
	
	public function getTest() {
		return $this->_test;
	}
	
	public function getCode() {
		return $this->_code;
	}
	
	public function getDuration() {
		return $this->_duration;
	}
	
	public function getException() {
		return $this->_exception;
	}
}