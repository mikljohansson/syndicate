<?php

/**
 * HTML Utility class.
 *
 * @static
 * @access		public
 * @package		synd.core.lib
 */
class SyndHTML {
	static function validateFile($data, &$errors, $index) {
		if (isset($data[$index])) {
			if (!empty($data[$index]['error'])) {
				switch ($data[$index]['error']) {
					case UPLOAD_ERR_NO_FILE:
						break;
					case UPLOAD_ERR_INI_SIZE:
					case UPLOAD_ERR_FORM_SIZE:
						$errors[$index]['msg'] = SyndLib::translate("Size of file too large (code %s).", $data[$index]['error']);
						break;
					case UPLOAD_ERR_PARTIAL:
						$errors[$index]['msg'] = SyndLib::translate("File not received correctly (code %s).", $data[$index]['error']);
						break;
					default: 
						$errors[$index]['msg'] = SyndLib::translate("Unknown file upload error (code %s).", $data[$index]['error']);
				}
			}
			else if (!is_uploaded_file($data[$index]['tmp_name']))
				$errors[$index]['msg'] = SyndLib::translate("Not an uploaded file '%s'", $data['DATA_FILE']['tmp_name']);
		}
		
		return empty($errors);
	}

	static function validateEmail($email) {
		  $domain = '(?:(?:[a-z0-9]|[a-z0-9][a-z0-9\-]*[a-z0-9])\.?)+';
		  $user = '[\w\-\.\+\^!#\$%&*+\/\=\?\`\|\{\}~\']+';
		  $ipv4 = '[0-9]{1,3}(\.[0-9]{1,3}){3}';
		  $ipv6 = '[0-9a-fA-F]{1,4}(\:[0-9a-fA-F]{1,4}){7}';
		  return preg_match("/^$user@($domain|(\[($ipv4|$ipv6)\]))$/", $email);
	}

	/**
	 * Filters evil and fixes some badly written HTML
	 *
	 * Filters scripts, applets etc. Closes open tags, eg <td> without ending </td>,
	 * adds /> to self-closing tags (img, br etc) and converts all tags to lowercase.
	 *
	 * @param	string	Code to filter
	 * @return	string
	 */
	static function filterHtml($text) {
		require_once 'core/lib/htmlfilter.php';

		$stripTags = array(false,
			'blink','object','meta','html','link',
			'frame','iframe','layer','ilayer',
			'plaintext');

		$removeTags = array(
			'script','style','applet','embed',
			'head','frameset','xml');

		$selfClosingTags =  array(
			'img','br','hr','input');

		$closeOpenTags = false;

		$removeAttr = array(
			'/^(?!a)/i' => array(
				'/target/i',
				),
			'/.*/' => array(
				'/^on.*/i',
				'/^dynsrc/i',
				'/^datasrc/i',
				'/^data.*/i',
				'/^lowsrc/i',
				),
			);

		$badAttr = array(
			'/.*/' => array(
				'/target/i' => array(
					array(
						'/^[\'"](?!_blank|_self|_parent|_top).*[\'"]$/i',
						),
					array(
						'"_self"',
						),
					),
				'/.*/' => array(
					array(
					  '/^([\'\"])\s*\S+\s*script\s*:(.*)([\'\"])/si',
					  '/^([\'\"])\s*https*\s+:(.*)([\'\"])/si',
					  '/^([\'\"])\s*mocha\s*:*(.*)([\'\"])/si',
					  '/^([\'\"])\s*about\s*:(.*)([\'\"])/si'
					  ),
					array(
					  '\\1invalid:\\2\\1',
					  '\\1invalid:\\2\\1',
					  '\\1invalid:\\2\\1',
					  '\\1invalid:\\2\\1'
					  )
					),
				  '/^style/i' =>array(
					array(
						'/expression/i',
						'/behaviou*r/i',
						'/binding/i',
						'/include-source/i',
						'/url\s*\(\s*([\'\"]*)\s*https*:.*([\'\"]*)\s*\)/si',
						'/url\s*\(\s*([\'\"]*)\s*\S+\s*script:.*([\'\"]*)\s*\)/si'
						),
					array(
						'invalid',
						'invalid',
						'invalid',
						'invalid',
						'url(\\1http://invalid/\\1)',
						'url(\\1http://invalid/\\1)'
						)
					)
				)
			);

		$addAttr = array();

		$text = sanitize($text, 
			$stripTags, $removeTags, $selfClosingTags, $closeOpenTags, 
			$removeAttr, $badAttr, $addAttr);
		return rtrim($text);
	}

	/**
	 * Highlight the rows differs between two strings.
	 *
	 * Uses the Tidy extension to format the code into lines and the 
	 * XDiff extension to get the unified diff
	 *
	 * @param	string	First (oldest) string
	 * @param	string	Second (newest) string
	 * @param	string	Class to use for highlighting
	 * @return	string
	 */
	static function highlightDiff($str1, $str2, $class = 'highlight') {
		if (!function_exists('xdiff_string_diff') && !SyndLib::dl('xdiff'))
			return $str2;

		$str1 = SyndHTML::tidyHtml($str1);
		$str2 = SyndHTML::tidyHtml($str2);
		
		if (null == ($diff = xdiff_string_diff($str1, $str2, 65000)))
			return $str2;
		$diff = preg_replace('/@@ .*? @@/', '', $diff);
		$diff = preg_replace_callback('/((^|\n)-[^\n]*)*(((^|\n)\+[^\n]*)+)/s', 
			create_function('$m',"return SyndHTML::highlightBlock(\$m[3],'$class');"), $diff);
		$diff = preg_replace('/(^|\n)\+|\+($|\n)/s', "\n", $diff);
		$diff = preg_replace('/((^|\n)\-[^\n]*)+/s', "<div class=\"$class\">&nbsp;</div>", $diff);
		
		return SyndHTML::tidyHtml($diff);
	}
	
	/**
	 * @access	private
	 */
	static function highlightBlock($str, $class) {
		$str = preg_replace('/(^|\n)[+\-\s]*<li[^>]*>(?!$)/is', "\n\\0<div class=\"$class\">", trim($str));
		if (!preg_match('/^[+\-\s]*<li[^>]*>/is', $str))
			return "<div class=\"$class\">".trim($str," \t\r\n+-")."</div>";
		return $str;
	}

	/**
	 * Runs code through the HtmlTidy extension.
	 * @param	string	Code to tidy up
	 * @return	string
	 */
	static function tidyHtml($str) {
		if (!function_exists('tidy_parse_string') && !SyndLib::dl('tidy'))
			return $str;

		tidy_load_config(tpl_design_path('core/lib/tidy.conf'));
		tidy_parse_string($str);
		tidy_clean_repair();

		return tidy_get_output();
	}

	/**
	 * Extracts the context of several words from several texts.
	 * 
	 * Useful for extracting content and highlighting matched words in
	 * search results.
	 *
	 * @param	mixed	String or array of strings making up full text
	 * @param	array	Words to highlight
	 * @param	integer	Approximate length of returned summary
	 * @param	integer	Approximate number of sections in summary
	 * @return	string
	 */
	static function getContextSummary($text, $terms, $length = 200, $maxContexts = 3) {
		if (is_array($text))
			$text = implode('. ', $text);
		
		// Characters used in english and national words
		$wordchars = "\wŠŽšžŸÀÁÂÃÄÅÇÈÉÊËÌÍÎÏÑÒÓÔÕÖØÙÚÛÜÝàáâãäåçèéêëìíîïñòóôõöøùúûüýÿÞþÐðßŒœÆæµ";
		
		// If no work highlighting, just return $length chars from the text and avoid 
		// splitting in the middle of a word.
		if (empty($terms))
			return preg_replace("/[{$wordchars}]+$/", '', substr($text, 0, $length+10));
		
		$terms = array_map('strtolower', $terms);
		$lower = strtolower($text);

		$offsets = array();
		foreach ($terms as $key => $term) {
			if (false !== ($i = strpos($lower, $term)))
				$offsets[$i] = $term;
		}
		
		// If terms weren't found, just return $length chars from the text and avoid 
		// splitting in the middle of a word.
		if (empty($offsets))
			return preg_replace("/[{$wordchars}]+$/", '', substr($text, 0, $length+10));
		
		ksort($offsets);
		$size = round(max(32, $length / count($offsets) / 2));

		$offset = 0;
		$contexts = array();
		$totalSize = 0;
		
		for ($i=0, $break=false; !$break && count($contexts) < $maxContexts && $i < 10; $i++) {
			foreach ($offsets as $term) {
				if (false !== ($index = strpos($lower, $term, $offset))) {
					if (preg_match('/\b(.{0,'.$size.'}.?\b'.preg_quote($term, '/').'.{0,'.$size.'}\b[.)\]!?]?)/is', 
						substr($text, max($index-$size*2,0), $size*4), $matches)) {
						
						$contexts[] = trim($matches[1]);
						$totalSize += strlen($matches[1]);
						if (count($contexts) >= $maxContexts || $totalSize >= $length)
							break 2;
						
						$offset = $index + $size + strlen($matches[1]) - strpos(strtolower($matches[1]), $term) - 1;
						if ($offset >= strlen($text))
							break 2;
					}

					$break = false;
				}
			}
		}
		
		if (empty($contexts)) {
			$offset = 0;
			foreach ($offsets as $term) {
				if (false !== ($index = strpos($lower, $term, $offset))) {
					$chunk = preg_replace("/^[^{$wordchars}]+|[^{$wordchars}]+$/", '', 
						substr($text, max($index-$size*2,$offset,0), $size*4));
						
					$contexts[] = $chunk;
					if (count($contexts) >= $maxContexts)
						break;

					$offset = $index + $size + strlen($chunk) - strpos(strtolower($chunk), $term) - 1;
					if ($offset >= strlen($text))
						break;
				}
			}
		}
		
		$contexts = preg_replace("/[^{$wordchars}.,?!;:'\"\/\\#%&\[\]()=+\-*@<>]+|^[^{$wordchars}<]+|[^{$wordchars}.!?>]+$|[#\s\-_.]{3,}/", ' ', $contexts);
		$contexts = preg_replace('/\b('.implode('|', 
			array_map('preg_quote', $terms))."){$wordchars}*/i", '<b>\0</b>', 
			array_map('htmlspecialchars', $contexts));

		return implode(' <b>..</b> ', $contexts);
	}
	
	static function translateNationalChars($text) {
		$text = strtr($text, 
		   'ŠŽšžŸÀÁÂÃÄÅÇÈÉÊËÌÍÎÏÑÒÓÔÕÖØÙÚÛÜÝàáâãäåçèéêëìíîïñòóôõöøùúûüýÿ',
		   'szszyaaaaaaceeeeiiiinoooooouuuuyaaaaaaceeeeiiiinoooooouuuuyy');
		$text = strtr($text, array(
			'Þ' => 'th', 'þ' => 'th', 'Ð' => 'dh', 'ð' => 'dh', 'ß' => 'ss',
   			'Œ' => 'oe', 'œ' => 'oe', 'Æ' => 'ae', 'æ' => 'ae', 'µ' => 'u'));
   		return $text;
	}
}
