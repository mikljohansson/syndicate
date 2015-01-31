<?php

/**
 * PRCE regexp expression to match word characters
 */
$SYND_WORDCHARS = "\w\dְֱֲֳִֵַָֹÊֻּֽ־ֿׁׂ׃װױײ״ÙÚÛÜÝאבגדהוחטיךכלםמןסעףפץצרשתûü‎ÿÞ‏׀נßֶזµ";

/**
 * Lowercase ISO-8858-1 characters
 */
$SYND_LOWERCHARS = "abcdefghijklmnopqrstuvwxyzאבגדהוחטיךכלםמןסעףפץצרשתûü‎ÿ‏נßזµ";

/**
 * Uppercase ISO-8858-1 characters
 */
$SYND_UPPERCHARS = "ABCDEFGHIJKLMNOPQRSTUVWXYZְֱֲֳִֵַָֹÊֻּֽ־ֿׁׂ׃װױײ״ÙÚÛÜÝÞ׀ßֶµ";

function synd_strtolower($s) {
	return strtr($s, $SYND_UPPERCHARS, $SYND_LOWERCHARS);
}
