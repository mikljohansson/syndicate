<?php
require_once 'core/i18n.inc';
require_once 'core/index/AbstractIndex.class.inc';
require_once 'core/index/IndexBuilder.class.inc';

class synd_parser_preg extends ForwardingIndexBuilder {
	function visitFragment($fragment, $field = null, $weight = 1) {
		global $SYND_TOKENIZE_SPLIT, $SYND_TOKENIZE_GLUE, $SYND_WORDCHARS;
		$split = $SYND_TOKENIZE_SPLIT;
		$glue = $SYND_TOKENIZE_GLUE;

//		$terms = preg_split("/([$split]|(?<=[$split])[$glue]|[$glue](?=[$split]))+/S", synd_strtolower($fragment), 8192, PREG_SPLIT_NO_EMPTY);

		$fragment = substr($fragment, 0, 65535);
		$matches = array();
		preg_match_all("/(?<![$SYND_WORDCHARS])(?:[^$split$glue]|(?<![$split$glue])[$glue](?![$split$glue]))+/", synd_strtolower($fragment), $matches);
		$terms = $matches[0];

		for ($i = 0, $l = count($terms); $i < $l && $i < 8192; $i++)
			$this->_next->visitTerm($terms[$i]);
		$this->_next->visitFragment($fragment, $field, $weight);
	}
}
