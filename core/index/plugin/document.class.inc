<?php
require_once 'core/i18n.inc';
require_once 'core/index/SyndIndexExtension.class.inc';
require_once 'core/index/IndexBuilder.class.inc';
require_once 'core/index/SyndDocument.class.inc';
require_once 'core/lib/TemplateLib.inc';

define('SYND_WEIGHT_TITLE', 1);
define('SYND_WEIGHT_HEADING', 0.8);
define('SYND_WEIGHT_KEYWORDS', 0.6);
define('SYND_WEIGHT_DESCRIPTION', 0.6);
define('SYND_WEIGHT_BODY', 0.4);

define('SYND_WEIGHT_DIRECTORY', 0.6);
define('SYND_WEIGHT_FILENAME', 0.8);
define('SYND_WEIGHT_FILETYPE', 0);

define('SYND_EXTRACT_LENGTH', 2592000);

/**
 * Uses various tools to extract text and meta data from files.
 *
 * pdftotext (http://public.planetmirror.com/pub/xpdf/) to extract PDF
 *  Set 'mapNumericCharNames no' in /etc/xpdfrc or garbage will result
 *  for some documents.
 *
 * ps2ascii to extract PDF (Not very good with national chars)
 *
 * antiword (http://www.winfield.demon.nl/) to extract Microsoft Word documents
 * catdoc (http://www.45.free.net/~vitus/ice/catdoc/) to extract Microsoft Word documents
 * xls2csv (http://www.45.free.net/~vitus/ice/catdoc/) to extract Microsoft Excel spreadsheets
 *
 * xlhtml (http://www.google.com/search?q=xlhtml) to extract Microsoft Excep spreadsheets
 * ppthtml (http://www.google.com/search?q=xlhtml) to extract Microsoft PowerPoint documents
 *
 * EXIF PHP extension (./configure --enable-exif) to extract EXIF data from JPEG and TIFF
 * Zip PHP extension (./configure --with-zip) to extract OpenOffice documents
 * libextractor (http://www.ovmj.org/libextractor/) to extract meta data
 *
 * GOCR (http://jocr.sourceforge.net/) for extracting text from images via Optical Character Recognition
 * Netpbm (http://netpbm.sourceforge.net/) for converting images to PNM for use with GOCR
 * djpeg for converting jpegs to PNM for use with GOCR
 *
 */
class synd_plugin_document extends SyndIndexExtension {
	protected $_extractors = array();
	
	function __construct() {
		$this->loadExtractor(new HtmlDocumentExtractor(
			array('text/html', 'text/xml', 'html', 'htm', 'xml', 'rss', 'php', 'jsp', 'asp', 'cfm')));
		
		if (null != ($path = SyndLib::which('pdftotext'))) {
			$this->loadExtractor(new ExternalDocumentExtractor(
				array('application/pdf', 'application/vnd.fdf', 'application/postscript', 'pdf', 'fdf', 'ps'),
				"$path -layout %s -", null));
		}
		else if (null != ($path = SyndLib::which('ps2ascii'))) {
			$this->loadExtractor(new ExternalDocumentExtractor(
				array('application/pdf', 'application/vnd.fdf', 'application/postscript', 'pdf', 'fdf', 'ps'),
				"$path -q -dSAFER %s", "$path -q -dSAFER"));
		}

		if (null != ($path = SyndLib::which('antiword'))) {
			$this->loadExtractor(new ExternalDocumentExtractor(
				array('application/msword', 'doc', 'dot'),
				"$path -s %s", "$path -s -"));
		}
		
		if (null != ($path = SyndLib::which('catdoc'))) {
			$this->loadExtractor(new ExternalDocumentExtractor(
				array('application/msword', 'text/rtf', 'doc', 'dot', 'rtf'),
				"$path -a -w %s", "$path -a -w"));
		}

		if (null != ($path = SyndLib::which('xls2csv'))) {
			$this->loadExtractor(new ExternalDocumentExtractor(
				array('application/msexcel', 'application/vnd.ms-excel', 'xls'),
				"$path -q0 -f\"%%Y%%m%%d\" %s", "$path -q0 -f\"%Y%m%d\""));
		}
		else if (null != ($path = SyndLib::which('xlhtml'))) {
			$this->loadExtractor(new ExternalDocumentExtractor(
				array('application/msexcel', 'application/vnd.ms-excel', 'xls'),
				"$path -xp:0-100 -asc  %s", null));
		}

		if (null != ($path = SyndLib::which('ppthtml'))) {
			$this->loadExtractor(new PowerPointExtractor(
				array('application/vnd.ms-powerpoint', 'ppt'),
				"$path %s", null));
		}

		$this->loadExtractor(new TextDocumentExtractor(
			array('text/plain', 'txt', 'ini', 'msg')));
		
		if (null != ($path = SyndLib::which('gocr'))) {
			if (null != ($path2 = SyndLib::which('jpegtopnm'))) {
				$this->loadExtractor(new ExternalDocumentExtractor(
					array('image/jpeg', 'jpg', 'jpeg'),
					"$path2 %s | gocr -", "$path2 | gocr -"));
			}
			else if (null != ($path2 = SyndLib::which('djpeg'))) {
				$this->loadExtractor(new ExternalDocumentExtractor(
					array('image/jpeg', 'jpg', 'jpeg'),
					"$path2 -pnm -grayscale %s | gocr -", "$path2 -pnm -grayscale | gocr -"));
			}

			if (null != ($path2 = SyndLib::which('tiftopnm'))) {
				$this->loadExtractor(new ExternalDocumentExtractor(
					array('image/tiff', 'tif', 'tiff'),
					"$path2 %s | gocr -", "$path2 | gocr -"));
			}

			if (null != ($path2 = SyndLib::which('giftopnm'))) {
				$this->loadExtractor(new ExternalDocumentExtractor(
					array('image/gif', 'gif'),
					"$path2 %s | gocr -", "$path2 | gocr -"));
			}

			if (null != ($path2 = SyndLib::which('pngtopnm'))) {
				$this->loadExtractor(new ExternalDocumentExtractor(
					array('image/png', 'png'),
					"$path2 %s | gocr -", "$path2 | gocr -"));
			}
		}

		// Nothing much of value in EXIF tags
		/*
		else if (function_exists('exif_read_data') || SyndLib::dl('exif')) {
			$this->loadExtractor(new ExifDocumentExtractor(
				array('image/jpeg', 'image/tiff', 'jpg', 'jpeg', 'tif', 'tiff')));
		}
		*/
		
		// "extract" mostly returns high-level document metadata
		/*
		if (null != ($path = SyndLib::which('extract')))
			$this->loadExtractor(new ExternalDocumentExtractor(array(), $path, null));
		*/
	}

	function initialize($index) {
		parent::initialize($index);
		$index->attachHook('builder', array($this, '_callback_builder'), AbstractIndex::PRIO_BUILDER_FILE);
	}
	
	function _callback_builder(&$result) {
		$result = new IndexDocumentExtractor($result, $this->_extractors);
	}		

	/**
	 * Load an extractor
	 * @param	DocumentExtractor
	 */
	function loadExtractor(DocumentExtractor $extractor) {
		$this->_extractors[] = $extractor;
	}
}

class IndexDocumentExtractor extends ForwardingIndexBuilder {
	protected $_extractors = null;
	
	function __construct(IndexBuilder $next, $extractors) {
		parent::__construct($next);
		$this->_extractors = $extractors;
	}
	
	function visitFile($file, $type = null, $buffer = null) {
		foreach ($this->_extractors as $extractor) {
			if ($extractor->visitFile($this->_next, $file, null == $type ? SyndLib::fileExtension($file) : $type, $buffer))
				break;
		}
	}
}

/**
 * Document extractor interface
 * @abstract
 * @access		public
 * @package		synd.core.index
 */
interface DocumentExtractor {
	/**
	 * Extract document title, content and structure
	 * @param	IndexBuilder	Builder to pass contents to
	 * @param	string			Path to file 
	 * @param	string			Mime type or file extension
	 * @param	string			Buffered contents
	 */
	function visitFile(IndexBuilder $indexer, $file, $type, $buffer = null);
}

/**
 * Extractor default implementation
 *
 * Be sure to override either getContentFromFile(), getContentFromBuffer() or
 * both or an inifitite loop will occur.
 *
 * @abstract
 */
class AbstractDocumentExtractor implements DocumentExtractor {
	protected $_types = null;
	
	function __construct($types) {
		$this->_types = array_flip($types);
	}
	
	/**
	 * Extract document content from file
	 *
	 * Returns file contents as string or null if file type is
	 * unsupported or extraction otherwise not possible.
	 *
	 * @param	string	Mime type or file extension
	 * @param	string	Path to file 
	 * @return	string
	 */
	function getContentFromFile($type, $file) {
		if (isset($this->_types[$type]) && null != ($buffer = file_get_contents($file)))
			return $this->getContentFromBuffer($type, $buffer);
		return null;
	}
	
	/**
	 * Extract document content from buffer
	 * @see		getContentFromFile()
	 * @param	string	Mime type or file extension
	 * @param	string	Buffered contents
	 * @return	string
	 */
	function getContentFromBuffer($type, $buffer) {
		if (!isset($this->_types[$type]))
			return null;
		
		$tmp = '/tmp/indexer-'.md5(uniqid(''));
		if (!SyndLib::file_put_contents($tmp, $buffer)) {
			trigger_error("Could not create temporary file '$tmp'.", E_USER_WARNING);
			return null;
		}
		
		$content = $this->getContentFromFile($type, $tmp);
		if (!@unlink($tmp))
			trigger_error("Could not delete temporary file '$tmp'.", E_USER_WARNING);

		return $content;
	}
	
	function visitFile(IndexBuilder $indexer, $file, $type, $buffer = null) {
		if (!isset($this->_types[$type]))
			return false;
		
		$content = null == $buffer ? 
			$this->getContentFromFile($type, $file) :
			$this->getContentFromBuffer($type, $buffer);

		if (false !== strpos($content, "\0")) {
			trigger_error('Binary file detected, skipping.', E_USER_WARNING);
			return false;
		}
		
		if (null == ($title = $this->getTitle(strlen($content) < 8192 ? 
			$content : substr($content, 0, 8192))))
			return false;
		
		$indexer->visitFragment($title, 'title', SYND_WEIGHT_TITLE);
		$indexer->visitFragment($content, null, SYND_WEIGHT_BODY);
		
		// Add directory, filename and filetype
		$uri = parse_url($file);
		if (isset($uri['path'])) {
			$indexer->visitFragment(dirname($uri['path']), null, SYND_WEIGHT_DIRECTORY);
			$indexer->visitFragment(SyndLib::chopExtension(basename($uri['path'])), null, SYND_WEIGHT_FILENAME);
			$indexer->visitFragment(SyndLib::fileExtension($uri['path']), 'filetype', SYND_WEIGHT_FILETYPE);
		}
		
		return true;
	}

	function getTitle($content) {
		// Normalize whitespace and text
		$chunk = ltrim(preg_replace('/[\x00-\x09\x0B-\x20\x93-\xA0]+|[.,;:\-+_* \t\r]{3,}|^[\-\s!?:;_.,»]+/S', ' ', $content));
		
		// Find optimal length of title
		if ((false === ($i = strpos($chunk, "\n", min(25, strlen($chunk)))) || $i > 80) &&
			(false === ($i = strpos($chunk, "\n")) || $i > 80) &&
			(false === ($i = strpos($chunk, ' ', min(60, strlen($chunk)))) || $i > 80))
			$i = 76;

		// Extract and normalize title
		return trim(preg_replace('/  +|[\-\s:;_,]$/', ' ', strtr(substr($chunk, 0, $i), "\n", ' ')));
	}
}

/**
 * Extracts structure from HTML documents
 */
class HtmlDocumentExtractor extends AbstractDocumentExtractor {
	function getContentFromFile($type, $file) {
		if (isset($this->_types[$type]))
			return tpl_strip_html_tags(file_get_contents($file));
		return null;
	}
	
	function getContentFromBuffer($type, $buffer) {
		if (isset($this->_types[$type]))
			return tpl_strip_html_tags($buffer);
		return null;
	}

	function visitFile(IndexBuilder $indexer, $file, $type, $buffer = null) {
		if (!isset($this->_types[$type]))
			return false;
		if (null == $buffer)
			$buffer = file_get_contents($file);
		
		$buffer = strtr(synd_html_entity_decode($buffer, ENT_QUOTES), ' ', ' ');
		if (false !== strpos($buffer, "\0")) {
			trigger_error('Binary file detected, skipping.', E_USER_WARNING);
			return false;
		}
		
		// Fold <img alt=""> into the text
		$content = preg_replace('/alt\s*=\s*"([^"<>]{1,512})"[^>]*>/iS', '\0 \1', $buffer);
		
		// Strip tags, <style> and <script> content
		$content = tpl_strip_html_tags(preg_replace('/<(style|script)[^>]*>.*?<\s*\/\1\s*>/isS', '', $content));

		// Extract title
		if ((!preg_match('/<title>([^<>]{4,2048})/iS', $buffer, $matches) ||
			null == ($title = $this->getTitle($matches[1]))) &&
			null == ($title = $this->getTitle($content)))
			return false;
		
		$indexer->visitFragment($title, 'title', SYND_WEIGHT_TITLE);
		$indexer->visitFragment(
			strlen($content) >= SYND_EXTRACT_LENGTH  ? 
			substr($content, 0, SYND_EXTRACT_LENGTH) : $content, 
			null, SYND_WEIGHT_BODY);

		// Add directory, filename and filetype
		$uri = parse_url($file);
		if (isset($uri['path'])) {
			$indexer->visitFragment(dirname($uri['path']), null, SYND_WEIGHT_DIRECTORY);
			$indexer->visitFragment(SyndLib::chopExtension(basename($uri['path'])), null, SYND_WEIGHT_FILENAME);
			$indexer->visitFragment(SyndLib::fileExtension($uri['path']), 'filetype', SYND_WEIGHT_FILETYPE);
		}
		
		// Extract meta description
		if (preg_match('/<meta [^<>]*?name="description"[^<>]*? content="([^"<>]{1,2048})"/iS', $buffer, $matches))
			$indexer->visitFragment($matches[1], 'description', SYND_WEIGHT_DESCRIPTION);

		// Extract meta keywords
		if (preg_match('/<meta [^<>]*?name="keywords"[^<>]*? content="([^"<>]{1,2048})"/iS', $buffer, $matches))
			$indexer->visitFragment($matches[1], 'subject', SYND_WEIGHT_KEYWORDS);

		// Extract link titles
		if (preg_match_all('/title\s*=\s*"([^"<>]{4,512})/iS', $buffer, $matches)) {
			foreach ($matches[1] as $match)
				$indexer->visitFragment($match, 'subject', SYND_WEIGHT_HEADING);
		}

		return true;
	}
}

/**
 * Simple text extractor
 */
class TextDocumentExtractor extends AbstractDocumentExtractor {
	function getContentFromFile($type, $file) {
		if (isset($this->_types[$type]) || preg_match('/^text\//', $type))
			return substr(file_get_contents($file), 0, SYND_EXTRACT_LENGTH);
		return null;
	}
	
	function getContentFromBuffer($type, $buffer) {
		if (isset($this->_types[$type]) || preg_match('/^text\//', $type))
			return substr($buffer, 0, SYND_EXTRACT_LENGTH);
		return null;
	}
}

/**
 * Executes an external program to extract document content
 */
class ExternalDocumentExtractor extends AbstractDocumentExtractor {
	protected $_storage = null;
	protected $_buffered = null;
	
	function __construct($types, $storage, $buffered) {
		assert('null != $storage || null != $buffered');
		parent::__construct($types);
		$this->_storage = $storage;
		$this->_buffered = $buffered;
	}
	
	function getContentFromFile($type, $file) {
		if (!isset($this->_types[$type]))
			return null;
		if (null == $this->_storage)
			return parent::getContentFromFile($type, $file);
		
		if (!preg_match('/^\w+:\/\//', $file)) 
			return $this->run(sprintf($this->_storage, escapeshellarg($file)));

		// Copy URI to temporary storage 
		$tmp = '/tmp/indexer-'.md5(uniqid(''));
		if (!copy($file, $tmp))
			return null;

		$content = $this->run(sprintf($this->_storage, escapeshellarg($tmp)));
		if (!unlink($tmp))
			return null;

		return $content;
	}
	
	function getContentFromBuffer($type, $buffer) {
		if (!isset($this->_types[$type]))
			return null;
		if (null == $this->_buffered)
			return parent::getContentFromBuffer($type, $buffer);
		return $this->run($this->_buffered, $buffer);
	}

	function run($cmd, $input = null) {
		return SyndLib::execute($cmd, $input, true, SYND_EXTRACT_LENGTH);
	}
}

/**
 * Strips temporary filename and 'Created with ppthtml' tags
 */
class PowerPointExtractor extends ExternalDocumentExtractor {
	function run($cmd, $input = null) {
		return tpl_strip_html_tags(preg_replace('/<(title|font|).*?<\/\1>/isS', '', parent::run($cmd, $input)));
	}
}

/**
 * Requires PHP to be configured with './configure --enable-exif'.If 
 * the exif extension is compiled as a shared library it will be 
 * automatically loaded when needed.
 */
class ExifDocumentExtractor extends AbstractDocumentExtractor {
	function getContentFromFile($type, $file) {
		$content = null;
		
		if (isset($this->_types[$type]) && 
			null != ($headers = @exif_read_data($file, 'EXIF', true, false))) {
			$content = implode(' ', array_filter($headers['EXIF'], 
				array('ExifDocumentExtractor', 'isString')));
			
			if (isset($headers['IFD0'])) {
				$content .= implode(' ', array_filter($headers['IFD0'], 
					array('ExifDocumentExtractor', 'isString')));
			}
		}
		
		return $content;
	}

	function isString($value) {
		return is_string($value) && !is_numeric(str_replace('/', '', $value));
	}
}
