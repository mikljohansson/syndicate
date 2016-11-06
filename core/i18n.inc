<?php

/**
 * PRCE regexp expression to match word characters
 */
$SYND_WORDCHARS = "\w\dŠŽšžŸÀÁÂÃÄÅÇÈÉÊËÌÍÎÏÑÒÓÔÕÖØÙÚÛÜÝàáâãäåçèéêëìíîïñòóôõöøùúûüýÿÞþÐðßŒœÆæµ";

/**
 * Lowercase ISO-8858-1 characters
 */
$SYND_LOWERCHARS = "abcdefghijklmnopqrstuvwxyzšžàáâãäåçèéêëìíîïñòóôõöøùúûüýÿþðßœæµ";

/**
 * Uppercase ISO-8858-1 characters
 */
$SYND_UPPERCHARS = "ABCDEFGHIJKLMNOPQRSTUVWXYZŠŽÀÁÂÃÄÅÇÈÉÊËÌÍÎÏÑÒÓÔÕÖØÙÚÛÜÝŸÞÐßŒÆµ";

/**
 * Regex character class to split tokens on
 */
$SYND_TOKENIZE_SPLIT = "\s<>*&|()\[\]{}\"'!,;?=";

/**
 * Regex character class that can join words
 */
$SYND_TOKENIZE_GLUE = "+\-.:\\/\\\\";

function synd_strtolower($s) {
	global $SYND_UPPERCHARS, $SYND_LOWERCHARS;
	return strtr($s, $SYND_UPPERCHARS, $SYND_LOWERCHARS);
}

/**
 * ISO-8859-1 compatible synd_htmlspecialchars
 * @link	http://php.net/manual/en/migration54.incompatible.php#115441
 */
function synd_htmlspecialchars($s) {
        return htmlspecialchars($s, ENT_COMPAT | ENT_HTML401 , 'ISO-8859-1');
}

