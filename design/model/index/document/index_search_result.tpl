<? require_once 'core/lib/SyndHTML.class.inc'; ?>
<div class="SearchResult">
	<?
	$uriParsed = parse_url($node->getLocation());
	$extension = strtolower(SyndLib::fileExtension($uriParsed['path']));

	// Include document template based on file extension
	if (null !== ($template = tpl_gui_path(get_class($node), "extensions/$extension.tpl", false)))
		include $template;
	else if ('rtf' == $extension) 
		include tpl_gui_path(get_class($node), "extensions/doc.tpl");
	else
		include tpl_gui_path(get_class($node), "extensions/default.tpl");

	// Chop document URI down to about 90 chars
	if (strlen($uri = $uriParsed['host'].urldecode($uriParsed['path'])) < 80) {
		if (isset($uriParsed['query']))
			$uri .= '?'.tpl_chop(urldecode($uriParsed['query']), 90 - strlen($uri));
	}
	else if (strlen($uriParsed['host']) < 80)
		$uri = $uriParsed['host'].'/.../'.strrev(dirname(substr(strrev(urldecode($uriParsed['path'])), 0, 80 - $length)));
	else
		$uri = tpl_chop($uriParsed['host'], 90);

	?>
	<div class="Abstract"><?= SyndHTML::getContextSummary(htmlspecialchars($node->getContents()), $highlight, 300) ?></div>
	<div class="Footer"><?= htmlspecialchars($uri) ?> -  - <a href="<?= 
		tpl_link('search',array('query'=>$request['query'],'rset'=>$rset,'docids'=>$docids)) 
		?>"><?= tpl_text('Similar pages') ?></a></div>
</div>
