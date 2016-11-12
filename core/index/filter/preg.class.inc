<?php
require_once 'core/i18n.inc';
require_once 'core/index/AbstractIndex.class.inc';
require_once 'core/index/IndexBuilder.class.inc';

class synd_parser_preg extends ForwardingIndexBuilder {
	function visitFragment($fragment, $field = null, $weight = 1) {
		global $SYND_WORDCHARS, $SYND_GLUECHARS, $SYND_TOKENIZE_GLUE;
		$word = $SYND_WORDCHARS;
		$glue = $SYND_TOKENIZE_GLUE;
		
		$terms = preg_split("/(?>[^$word$glue]+)/S", synd_strtolower($fragment), 8192, PREG_SPLIT_NO_EMPTY);
		for ($i = 0, $l = count($terms); $i < $l && $i < 8192; $i++) {
			$term = substr(trim($terms[$i], $SYND_GLUECHARS), 0, AbstractIndex::MAX_TERM_LENGTH);
			if ($term != '') {
				$this->_next->visitTerm($term);
			}
		}

		$this->_next->visitFragment($fragment, $field, $weight);
	}
}
