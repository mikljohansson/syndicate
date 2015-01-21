<?php
/**
 * SyndRTE PHP loader
 *
 * Put this into your page:
 *	<script defer="defer" type="text/javascript" src="/path/to/syndrte/loader.php"></script>
 *  <link rel="stylesheet" type="text/css" href=" src="/path/to/syndrte/style.css" />
 *
 * You will also need to include the code from toolbar/toolbar.html
 * somewhere into the body of your page, it contains the html for 
 * the toolbars. PHP example:
 *
 * <? print include("/path/to/syndrte/toolbar/toolbar.inc"); ?>
 *
 * This method is a bit harder to implement than using the pure javascript
 * loader (loader.js) but it results in much shorter editor startup times.
 *
 * @access		public
 * @package		synd.rte
 */

// Try to enable output compression (nice for people on modem)
if (!ini_get('zlib.output_compression') && function_exists('ob_gzhandler'))
	ob_start('ob_gzhandler');

header('Cache-Control: private');
header('Content-Type: application/x-javascript');

// Detect browser
if (stristr($_SERVER['HTTP_USER_AGENT'], 'Gecko')) {
	$api = array(
		'script/core.js',
		'script/gui.js',
		'script/gecko/eDOM/eDOM.js',
		'script/gecko/eDOM/eDOMXHTML.js',
		'script/gecko/eDOM/domlevel3.js',
		'script/gecko/eDOM/mozCE.js',
		'script/gecko/Mozile/mozileWrappers.js',
		'script/gecko/api.js',);
}
else if (preg_match('/MSIE (\d)/', $_SERVER['HTTP_USER_AGENT'], $matches) && $matches[1] >= 6) {
	$api = array(
		'script/core.js', 
		'script/gui.js',
		'script/ie/api.js',);
}
else
	return null;

// Include all javascript code
foreach ($api as $file) 
	print file_get_contents($file)."\r\n";

?>

// Load the editor
new SyndEditor(document);