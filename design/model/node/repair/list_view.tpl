<div class="Item">
	<div class="Header">
		<h4><a href="<?= tpl_link($node->getHandler(),$node->objectId()) ?>"><?= $node->getTitle() ?></a></h4>
	</div>
	<div class="Info">
		<?
		print tpl_text('Created on %s', ucwords(tpl_strftime('%d %b %Y', $node->data['TS_CREATE'])));
		if ($node->isClosed())
			print ', '.tpl_text('Closed %s', ucwords(tpl_strftime('%d %b %Y', $node->data['TS_RESOLVE'])));
		if ($node->data['FLAG_NO_WARRANTY'])
			print ', <span class="Notice">'.tpl_text('Not warranty').'</span>';
		?>
	</div>
	<div class="Abstract">
		<? if (isset($highlight)) { ?>
			<? 
			require_once 'core/lib/SyndHTML.class.inc';
			print tpl_def(SyndHTML::getContextSummary($node->data['INFO_HEAD'], $highlight), $node->data['INFO_HEAD']);
			?>
		<? } else { ?>
			<?= tpl_chop($node->getDescription(),300) ?>
		<? } ?>
	</div>
</div>
