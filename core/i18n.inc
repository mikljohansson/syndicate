<?php

/**
 * PRCE regexp expression to match word characters
 */
$SYND_WORDCHARS = "\w\d����������������������������������������������������������������ߌ���";

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