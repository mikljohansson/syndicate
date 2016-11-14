<?php
require_once 'core/i18n.inc';
require_once 'core/index/AbstractIndex.class.inc';
require_once 'core/index/IndexBuilder.class.inc';

class synd_parser_preg extends ForwardingIndexBuilder {
	function visitFragment($fragment, $field = null, $weight = 1) {
		global $SYND_WORDCHARS, $SYND_GLUECHARS, $SYND_TOKENIZE_GLUE;
		$word = $SYND_WORDCHARS;
		$glue = $SYND_TOKENIZE_GLUE;
		
		$terms = preg_split("/[^$word$glue]+/S", synd_strtolower(substr($fragment, 0, 65535)), 8192, PREG_SPLIT_NO_EMPTY);
		foreach ($terms as $term) {
			$term = substr(trim($term, $SYND_GLUECHARS), 0, AbstractIndex::MAX_TERM_LENGTH);
			if ($term != '') {
				$this->_next->visitTerm($term);
			}
		}

		$this->_next->visitFragment($fragment, $field, $weight);
	}
}
