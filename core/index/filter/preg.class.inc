<?php
require_once 'core/i18n.inc';
require_once 'core/index/IndexBuilder.class.inc';

class synd_parser_preg extends ForwardingIndexBuilder {
	function visitFragment($fragment, $field = null, $weight = 1) {
		global $SYND_TOKENIZE_SPLIT, $SYND_TOKENIZE_GLUE;
		$split = $SYND_TOKENIZE_SPLIT;
		$glue = $SYND_TOKENIZE_GLUE;

		$terms = preg_split("/([$split]|(?<=[$split])[$glue]|[$glue](?=[$split]))+/S", synd_strtolower($fragment), 8192, PREG_SPLIT_NO_EMPTY);
		foreach ($terms as $term)
			$this->_next->visitTerm($term);
		$this->_next->visitFragment($fragment, $field, $weight);
	}
}
