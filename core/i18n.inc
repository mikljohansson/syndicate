<?php

/**
 * PRCE regexp expression to match word characters
 */
$SYND_WORDCHARS = "\w\d����������������������������������������������������������������ߌ���";

/**
 * Characters that can join words
 */
$SYND_GLUECHARS = "+-.:/\\@'";
$SYND_TOKENIZE_GLUE = str_replace('/', '\\/', preg_quote($SYND_GLUECHARS));

/**
 * Lowercase ISO-8858-1 characters
 */
$SYND_LOWERCHARS = "abcdefghijklmnopqrstuvwxyz��������������������������������ߜ�";

/**
 * Uppercase ISO-8858-1 characters
 */
$SYND_UPPERCHARS = "ABCDEFGHIJKLMNOPQRSTUVWXYZ����������������������������ݟ��ߌƵ";

function synd_strtolower($s) {
	global $SYND_UPPERCHARS, $SYND_LOWERCHARS;
	return strtr($s, $SYND_UPPERCHARS, $SYND_LOWERCHARS);
}

/**
 * ISO-8859-1 compatible synd_htmlspecialchars
 * @link	http://php.net/manual/en/migration54.incompatible.php#115441
 */
function synd_htmlspecialchars($s, $flags = null, $encoding = null, $double_encode = true) {
        return htmlspecialchars($s, 
		$flags !== null ? $flags : ENT_COMPAT|ENT_HTML401, 
		$encoding !== null ? $encoding : ini_get('default_charset'), 
		$double_encode);
}

function synd_html_entity_decode($s, $flags = null, $encoding = null) {
	return html_entity_decode($s, 
		$flags !== null ? $flags : ENT_COMPAT|ENT_HTML401, 
		$encoding !== null ? $encoding : ini_get('default_charset'));
}
