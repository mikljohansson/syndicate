<?php

/**
 * PRCE regexp expression to match word characters
 */
$SYND_WORDCHARS = "\w\dְֱֲֳִֵַָֹÊֻּֽ־ֿׁׂ׃װױײ״ÙÚÛÜÝאבגדהוחטיךכלםמןסעףפץצרשתûü‎ÿÞ‏׀נßֶזµ";

/**
 * Characters that can join words
 */
$SYND_GLUECHARS = "+-.:/\\@";
$SYND_TOKENIZE_GLUE = str_replace('/', '\\/', preg_quote($SYND_GLUECHARS));

/**
 * Lowercase ISO-8858-1 characters
 */
$SYND_LOWERCHARS = "abcdefghijklmnopqrstuvwxyzאבגדהוחטיךכלםמןסעףפץצרשתûü‎ÿ‏נßזµ";

/**
 * Uppercase ISO-8858-1 characters
 */
$SYND_UPPERCHARS = "ABCDEFGHIJKLMNOPQRSTUVWXYZְֱֲֳִֵַָֹÊֻּֽ־ֿׁׂ׃װױײ״ÙÚÛÜÝÞ׀ßֶµ";

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

