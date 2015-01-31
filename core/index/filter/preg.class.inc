<?php
require_once 'core/defines.inc';
require_once 'core/index/IndexBuilder.class.inc';

class synd_parser_preg extends ForwardingIndexBuilder {
	function visitFragment($fragment, $field = null, $weight = 1) {
		$terms = preg_split("/[^{$SYND_WORDCHARS}]+/S", strtolower($fragment), null, PREG_SPLIT_NO_EMPTY);
		foreach ($terms as $term)
			$this->_next->visitTerm($term);
		$this->_next->visitFragment($fragment, $field, $weight);
	}
}
