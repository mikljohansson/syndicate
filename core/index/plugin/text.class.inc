<?php
require_once 'core/i18n.inc';
require_once 'core/index/SyndIndexExtension.class.inc';
require_once 'core/index/IndexBuilder.class.inc';
require_once 'core/lib/TemplateLib.inc';

/**
 * Text filter extension.
 *
 * Strips html and odd characters from documents and queries. 
 * Translates national characters into their english equivalents.
 *
 */
class synd_plugin_text extends SyndIndexExtension {
	function initialize($index) {
		parent::initialize($index);
		$index->attachHook('text_strip_query', array($this, '_callback_text_strip_query'), 20);
		$index->attachHook('query_process_term', array($this, '_callback_query_process_term'), 10);
		$index->attachHook('document_fragment', array($this, '_callback_document_fragment'), 20);
		$index->attachHook('builder', array($this, '_callback_builder'), AbstractIndex::PRIO_BUILDER_FRAGMENT); 
	}
	
	static function normalize($text) {
		return synd_html_entity_decode($text, ENT_QUOTES);
	}

	function _callback_query_process_term(&$result, &$term) {
		$term = synd_strtolower(self::normalize($term));
	}

	function _callback_text_strip_query(&$result, &$text) {
		$text = preg_replace('/\bAND\b/', '&&', $text);
		$text = preg_replace('/\bOR\b/', '||', $text);
		$text = preg_replace('/\bNOT\b/', '-', $text);
		$text = self::normalize($text);
	}
	
	function _callback_document_fragment(&$result, $document) {
		$document->setText(self::process($document->toString()));
	}

        function _callback_builder(&$result) {
                $result = new synd_plugin_text_filter($result);
        }

	static function process($fragment) {
		// Keep href'ed urls
		$fragment = preg_replace('/<a .*?href=(")?(.*)(?(1)"|\s).*?>/s', ' \2 ', $fragment);
		return synd_strtolower(self::normalize(tpl_strip_html_tags($fragment)));
	}
}

class synd_plugin_text_filter extends ForwardingIndexBuilder {
        function __construct(IndexBuilder $next) {
                parent::__construct($next);
        }

        function visitFragment($fragment, $field = null, $weight = 1) {
                $this->_next->visitFragment(synd_plugin_text::process($fragment), $field, $weight);
        }
}

