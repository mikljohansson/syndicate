<?php
require_once 'core/i18n.inc';
require_once 'core/index/SyndIndexExtension.class.inc';
require_once 'core/index/IndexBuilder.class.inc';

/**
 * Word stemmer 
 *
 * Stems words in documents and queries to improve precision and
 * reduce index size. Words are chopped of their endings leaving 
 * only the word-stem. 
 *
 * @package		synd.core.index
 * @link		http://cvs.php.net/viewvc.cgi/pecl/stem/README?view=co
 * @see			SyndDLE
 */
class synd_plugin_stem extends SyndIndexExtension {
	var $_language = null;

	/**
	 * Available language constants as of writing
	 *
	 * STEM_DANISH
	 * STEM_DUTCH
	 * STEM_ENGLISH -- enhanced Porter stemming algorithm
	 * STEM_FINNISH
	 * STEM_FRENCH
	 * STEM_GERMAN
	 * STEM_ITALIAN
	 * STEM_NORWEGIAN
	 * STEM_PORTER -- the original Porter stemming algorithm
	 * STEM_PORTUGUESE
	 * STEM_RUSSIAN
	 * STEM_RUSSIAN_UNICODE -- text is in Unicode encoding rather than UTF-8
	 * STEM_SPANISH
	 * STEM_SWEDISH
	 *
	 * @param	int	Language to stem
	 */
	function __construct($language) {
		$this->_language = $language;
	}

	function initialize($index) {
		parent::initialize($index);
		$index->attachHook('query_visit_term', array($this, '_callback_query_visit_term'), 20);
		$index->attachHook('query_process_term', array($this, '_callback_query_process_term'), 20);
		$index->attachHook('index_posting', array($this, '_callback_index_posting'), 30);
		$index->attachHook('builder', array($this, '_callback_builder'), AbstractIndex::PRIO_BUILDER_TERM);
	}
	
	function _callback_query_process_term(&$result, &$term) {
		$term = self::process($term, $this->_language);
	}
	
	function _callback_query_visit_term(&$result, $builder, $expression) {
		global $SYND_WORDCHARS;
		$expression->setString(preg_replace_callback("/[{$SYND_WORDCHARS}]+/", 
			array($this,'_callback_stem'), $expression->toString()));
	}

	function _callback_stem($matches) {
		return self::process($matches[0], $this->_language);
	}

	/**
	 * Stems the term
	 */
	function _callback_index_posting($result, $docid, &$term) {
		profiler_enter(__METHOD__);
		$term = self::process($term, $this->_language);
		profiler_leave(__METHOD__);
	}

	function _callback_builder(&$result) {
		$result = new StemmingFilter($result, $this->_language);
	}

	static function process($term, $language) {
		if (strlen($term) < 16 && ctype_alpha($term))
			return stem($term, $language);
		return $term;
	}
}

class StemmingFilter extends ForwardingIndexBuilder {
	protected $_language = null;
	
	function __construct(IndexBuilder $next, $language) {
		parent::__construct($next);
		$this->_language = $language;
	}

	function visitTerm($term, $field = null) {
		$this->_next->visitTerm(synd_plugin_stem::process($term, $this->_language), $field);
	}
}
