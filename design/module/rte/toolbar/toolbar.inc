<?php
/**
 * SyndRTE PHP toolbar loader
 *
 * Use in your page like:
 * <code>
 *  <? print include('syndrte/toolbar/toolbar.inc'); ?>
 * </code>
 *
 * or in your function like:
 * <code>
 *  $rteToolbar = include('syndrte/toolbar/toolbar.inc');
 * </code>
 *
 * @access		public
 * @package		synd.core.module
 * @return		string
 */

$base = substr(realpath(dirname(__FILE__).'/..'), strlen(rtrim($_SERVER['DOCUMENT_ROOT'],DIRECTORY_SEPARATOR))).'/';
$text = file_get_contents(dirname(__FILE__).'/toolbar.html');
$text = str_replace('{base}', $base, $text);

// Button toolbar
$text = str_replace('{btnLeft}', isset($_COOKIE['srte_toolbar_button']['x']) ? 
	$_COOKIE['srte_toolbar_button']['x'] : '10px', $text);
$text = str_replace('{btnTop}', isset($_COOKIE['srte_toolbar_button']['y']) ? 
	$_COOKIE['srte_toolbar_button']['y'] : '10px', $text);

// Properties inspector toolbar
$text = str_replace('{propLeft}', isset($_COOKIE['srte_toolbar_properties']['x']) ? 
	$_COOKIE['srte_toolbar_properties']['x'] : '10px', $text);
$text = str_replace('{propTop}', isset($_COOKIE['srte_toolbar_properties']['y']) ? 
	$_COOKIE['srte_toolbar_properties']['y'] : '45px', $text);
$text = str_replace('{propVisibility}', isset($_COOKIE['srte_toolbar_properties']['vis']) ? 
	$_COOKIE['srte_toolbar_properties']['vis'] : 'hidden', $text);

// DOM inspector toolbar
$text = str_replace('{domLeft}', isset($_COOKIE['srte_toolbar_dom']['x']) ? 
	$_COOKIE['srte_toolbar_dom']['x'] : '10px', $text);
$text = str_replace('{domTop}', isset($_COOKIE['srte_toolbar_dom']['y']) ? 
	$_COOKIE['srte_toolbar_dom']['y'] : '95%', $text);
$text = str_replace('{domVisibility}', isset($_COOKIE['srte_toolbar_dom']['vis']) ? 
	$_COOKIE['srte_toolbar_dom']['vis'] : 'hidden', $text);

// Image toolbar
$text = str_replace('{imageLeft}', isset($_COOKIE['srte_toolbar_image']['x']) ? 
	$_COOKIE['srte_toolbar_image']['x'] : '10px', $text);
$text = str_replace('{imageTop}', isset($_COOKIE['srte_toolbar_image']['y']) ? 
	$_COOKIE['srte_toolbar_image']['y'] : '100px', $text);

return $text;
