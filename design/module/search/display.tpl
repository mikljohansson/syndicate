<?
if (preg_match_all('/\w+/', $request['query'], $matches))
	$content = preg_replace('/\b('.implode('|', $matches[0]).')/i', '<em>\0</em>', $document['CONTENT']);
else
	$content = $document['CONTENT'];

function _callback_replace_page() {
	static $page = 2;
	return '<div class="Pagination">'.tpl_text('Page %d', $page++).'</div>';
}

if (($count = substr_count($content, '')) > 1) 
	$content = preg_replace_callback('/\x0C/', '_callback_replace_page', $content);

?>
<div class="CachedPage">
	<div class="Notice">
		<? 
		print tpl_translate('This is the HTML version of the file <a href="%s">%s</a> as seen on <em>%s</em>',
			$document['URI'], $document['URI'], date('Y-m-d', $document['REVISIT']-$document['TTL'])); 
		if (!empty($document['MODIFIED']))
			print tpl_text(', the document was last modified <em>%s</em>.', 
			date('Y-m-d', $document['MODIFIED']));
		?>
	</div>
	<div class="Content">
		<? if ($count > 1) { ?>
		<div class="Pagination"><?= tpl_text('Page %d', 1) ?></div>
		<? } ?>
		<?= tpl_html_format($content) ?>
	</div>
</div>
