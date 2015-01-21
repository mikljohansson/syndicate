<?php
/**
 * Printing library functions
 * @package		synd.core.lib
 * @filesource
 */
require_once 'core/lib/SyndLib.class.inc';

/**
 * Printing utility class.
 *
 * @static
 * @access		public
 * @package		synd.core.lib
 */
class SyndPrint {
	static function printTemplate($printer, $file, $filter) {
		switch (strtolower(SyndLib::fileExtension($file))) {
			case 'ps':
			case 'prn':
				SyndPrint::psPrint($filter, $file, $filter);
				break;
			case 'pdf':
				SyndPrint::pdfPrint($filter, $file, $filter);
				break;
			default:
				trigger_error('Unknown file extension ('.SyndLib::fileExtension($file).')', E_USER_WARNING);
		}
	}
	
	/**
	 * Knows how to apply a set of filters to a postscript file and 
	 * print it. Useful for making postscript reciept templates and 
	 * using the filters to fill them with data before sending to a
	 * networked printer nearby the client.
	 *
	 * <code>
	 * Filters have the form of 
	 * array(
	 *  'NAME' => 'Mikael Johansson',
	 *  'DATE' => '2004-04-04',
	 *  'LIST' => array(
	 *    0 => array('SERIAL' => '123', 'MAKE' => 'IBM')
	 *    1 => array('SERIAL' => '456', 'MAKE' => 'Toshiba')))
	 *
	 * Resulting in the textstrings
	 *  NAME, DATE, 
	 *  LIST_0_SERIAL, LIST_0_MAKE
	 *  LIST_1_SERIAL, LIST_1_MAKE
	 * being replaced with their corresponding values. 
	 *
	 *
	 * To make table generating easier the filtering will also replace 
	 * LIST_X_SERIAL etc with spaces so any table rows not filled with 
	 * data won't display like:
	 *  LIST_3_SERIAL	LIST_3_MAKE
	 *  LIST_4_SERIAL	LIST_4_MAKE
	 *  LIST_5_SERIAL	LIST_5_MAKE
	 *
	 *
	 * Keys can be padded to any length like:
	 *	NAME_PADPADPADPADPADPADPADPADPAD
	 *	LIST_0_MAKE_PADPADPADPADPADPADPAD
	 * </code>
	 *
	 * @param	string	Printer to print to
	 * @param	string	Path to template file
	 * @param	array	Filters to apply to template
	 * @return	bool
	 */
	static function psPrint($printer, $file, $filter = null) {
		$template = file_get_contents($file);
		$postscript = null != $filter ? SyndPrint::psFilter($template, $filter) : $template;
		return SyndPrint::_print($printer, $postscript);
	}
	
	/**
	 * @access	private
	 * @return	bool
	 */
	static function _print($printer, $buffer) {
		if (null == ($lpr = SyndLib::which('lpr'))) {
			trigger_error("lpr does not exist or is not executable.", E_USER_ERROR);
			return false;
		}
		
		$output = SyndLib::execute("$lpr -P $printer", $buffer);
		if ('' != $output) {
			trigger_error("Failed to print, LPR said '".substr($output,0,256)."'", E_USER_WARNING);
			return false;
		}
		
		return true;
	}
	
	/**
	 * @access	private
	 */
	static function psFilter($buffer, $filter, $_prefix = null) {
		global $_syndprint_value;
		
		$stripExcessNum = false;
		foreach ($filter as $key => $value) {
			if (is_int($key))
				$stripExcessNum = true;
			if (is_array($value))
				$buffer = SyndPrint::psFilter($buffer, $value, preg_quote($_prefix, '/').$key.'_');
			else if (strlen($value)) {
				$key = preg_quote($key, '/');
				$_syndprint_value = $value;
				$buffer = preg_replace_callback("/(?<!\w){$_prefix}{$key}_*(_*PAD)*(?=[^\w])/", 
					array('SyndPrint', '_psEscape'), $buffer);
			}
			else {
				$buffer = preg_replace("/(?<!\w){$_prefix}{$key}_*(_*PAD)*(?=[^\w])/", ' ', $buffer);
			}
		}
		
		// Replace any table cells not filled with data, ie ROW_12_SERIAL_PADPAD
		if ($stripExcessNum)
			$buffer = preg_replace("/(?<!\w){$_prefix}\d+_[\w\-]+/", ' ', $buffer);
		if (null != $_prefix)
			$buffer = preg_replace("/(?<!\w){$_prefix}[\w\-]+/", ' ', $buffer);
		
		return $buffer;
	}
	
	/**
	 * @access	private
	 */
	static function _psEscape($matches) {
		global $_syndprint_value;
		$result = SyndPrint::_psQuote(substr($_syndprint_value, 0, strlen($matches[0])));
		$_syndprint_value = substr($_syndprint_value, strlen($matches[0]));
		return $result;
	}
	
	/**
	 * @access	private
	 */
	static function _psQuote($value) {
		$filter = array(
//			"\r\n" => '\n',
//			"\n" => '\n',
			"\r\n" => ' ',
			"\n" => ' ',
			"\t" => '\t',
			"\b" => '\b',
			"\f" => '\f',
			"\\" => '\\\\',
			"(" => '\(',
			")" => '\)',);
		$value = strtr($value, $filter);
		$value = preg_replace('/[\r\n\t]+(?=\s)/', ' ', $value);
		return $value;
	}

	/**
	 * @param	string	Printer to print to
	 * @param	string	Path to template file
	 * @param	array	Filters to apply to template
	 * @return	bool
	 */
	static function pdfPrint($printer, $file, $filter = null) {
		$buffer = null != $filter ? SyndPrint::pdfMerge($file, $filter) : file_get_contents($file);
		return SyndPrint::_print($printer, $buffer);
	}
	
	/**
	 * Merges a PDF with form values
	 * @param	string	Path to PDF template
	 * @param	array	Form values to fill in
	 * @return	string
	 */
	static function pdfMerge($file, $filter) {
		if (null == ($pdftk = SyndLib::which('pdftk')))
			throw new RuntimeException('pdftk does not exist or is not executable');
		if (false === ($input = file_get_contents($file)))
			return null;
		
		$fdf = SyndPrint::fdfGenerate($filter, array());
		$tmp = SyndLib::mktemp('fdf');
		if (!SyndLib::file_put_contents($tmp, $fdf)) 
			return null;

		$cmd = "$pdftk - fill_form ".escapeshellarg($tmp)." output - flatten";
		$result = SyndLib::execute($cmd, $input);
		
		unlink($tmp);
		return $result;
	}
	
	/**
	 * Creates an FDF from a set of form values
	 * @param	array	Textfields and other open ended fields array(string:name => mixed:value)
	 * @param	array	Checkboxes and radiobuttons array(string:name => boolean:selected)
	 * @param	string	Uri of PDF
	 * @return	string
	 */
	static function fdfGenerate($values, $names, $uri = null) {
		$fdf  = "%FDF-1.2\x0d%\xe2\xe3\xcf\xd3\x0d\x0a";
		$fdf .= "1 0 obj\x0d<< ";	// Open the Root dictionary
		$fdf .= "\x0d/FDF << ";		// Open the FDF dictionary
		$fdf .= "/Fields [ ";		// Open the form Fields array

	    $fdf .= SyndPrint::fdfImplode(SyndPrint::pdfEscape($values));
	    $fdf .= SyndPrint::fdfImplode(SyndPrint::pdfEscapeName($names));

		$fdf .= "] \x0d"; 			// Close the Fields array
	    
	    if (null != $uri)
	    	$fdf .= "/F ".SyndPrint::pdfEscape($uri)." \x0d";

		$fdf .= ">> \x0d";				// Close the FDF dictionary
		$fdf .= ">> \x0dendobj\x0d";	// Close the Root dictionary

		$fdf .= "trailer\x0d<<\x0d/Root 1 0 R \x0d\x0d>>\x0d";
		$fdf .= "%%EOF\x0d\x0a";
		
		return $fdf;
	}
	
	/**
	 * Implodes the FDF fields
	 * @access	private
	 */
	static function fdfImplode($values, $_key = '') {
		if ('' != $_key)
			$_key .= '.';
		$fdf = '';
		
		foreach ($values as $key => $value) {
			$fdf .= '<< '; // Open dictionary

			if (is_array($value)) {
				$fdf .= "/T ".SyndPrint::pdfEscape($key)." ";
				$fdf .= "/Kids [ ";
				$fdf .= SyndPrint::fdfImplode($value, $_key.$key);
				$fdf .= "] ";
			}
			else {
				$fdf .= "/T ".SyndPrint::pdfEscape($key)." ";
				$fdf .= "/V ".$value." ";

				$fdf .= "/ClrF 2 ";		// Not hidden
				$fdf .= "/ClrFf 1 ";	// Not read-only
			}

			$fdf .= ">> \x0d"; // Close dictionary
		}
		
		return $fdf;
	}
	
	/**
	 * Escapes using backslash and \Octal
	 * @access	private
	 */
	static function pdfEscape($value) {
		if (is_array($value)) {
			$result = array();
			foreach ($value as $key => $subvalue)
				$result[$key] = SyndPrint::pdfEscape($subvalue);
			return $result;
		}
		return '('.preg_replace_callback('/[\\\\()\x00-\x1F\x7F-\xFF]/', array('SyndPrint','_escape'), (string)$value).')';
	}
	
	/**
	 * @access	private
	 */
	static function _escape($matches) {
		if ('\\' == $matches[0]{0} || '(' == $matches[0]{0} || ')' == $matches[0]{0})
			return '\\'.$matches[0]{0};
		return sprintf('\\%03o', ord($matches[0]{0}));
	}

	/**
	 * Escapes using #Hexcode
	 * @access	private
	 */
	static function pdfEscapeName($value) {
		if (is_array($value)) {
			$result = array();
			foreach ($value as $key => $subvalue)
				$result[$key] = SyndPrint::pdfEscapeName($subvalue);
			return $result;
		}
		if (is_bool($value))
			return $value ? '/Yes' : '/Off';
		return '/'.preg_replace_callback('/[#\x00-\x20\x7F-\xFF]/', array('SyndPrint','_nameEscape'), (string)$value);
	}
	
	/**
	 * @access	private
	 */
	static function _nameEscape($matches) {
		return sprintf('#%02x', ord($matches[0]{0}));
	}
}
