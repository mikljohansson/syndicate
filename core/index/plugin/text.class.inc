<?php
require_once 'core/index/SyndIndexExtension.class.inc';

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
	}
	
	function normalize($text) {
		// Translate national chars
		$text = strtr(html_entity_decode($text, ENT_QUOTES), 
		   'ŠŽšžŸÀÁÂÃÄÅÇÈÉÊËÌÍÎÏÑÒÓÔÕÖØÙÚÛÜÝàáâãçèéêëìíîïñòóôõøùúûüýÿµ',
		   'szszyaaaaäåceeeeiiiinooooöouuuuyaaaaceeeeiiiinooooouuuuyyu');
		return strtr($text, array(
			'Þ' => 'th', 'þ' => 'th', 'Ð' => 'dh', 'ð' => 'dh', 'ß' => 'ss',
   			'Œ' => 'oe', 'œ' => 'oe', 'Æ' => 'ae', 'æ' => 'ae'));
	}

	function _callback_query_process_term(&$result, &$term) {
		$term = strtolower($this->normalize($term));
	}

	function _callback_text_strip_query(&$result, &$text) {
		$text = preg_replace('/\bAND\b/', '&&', $text);
		$text = preg_replace('/\bOR\b/', '||', $text);
		$text = preg_replace('/\bNOT\b/', '-', $text);
		$text = $this->normalize($text);
	}
	
	function _callback_document_fragment(&$result, $document) {
		// Keep href'ed urls
		$text = preg_replace('/<a .*?href=(")?(.*)(?(1)"|\s).*?>/s', ' \2 ', $document->toString());
		$document->setText(strtolower($this->normalize(strip_tags($text))));
	}
}
