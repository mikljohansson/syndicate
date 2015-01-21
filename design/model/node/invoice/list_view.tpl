<div class="Item">
	<div class="Header">
		<h3><a href="<?= tpl_link($node->getHandler(),$node->objectId()) ?>"><? 
		if (!isset($highlight)) 
			print $node->data['INFO_HEAD'];
		else {
			require_once 'core/lib/SyndHTML.class.inc';
			print tpl_def(SyndHTML::getContextSummary($node->data['INFO_HEAD'], $highlight), $node->data['INFO_HEAD']);
		}
		?></a></h3>
	</div>
	<div class="Info">
		<?= tpl_text('Created on %s', ucwords(tpl_strftime('%d %b %Y', $node->data['TS_CREATE']))) ?>, 
		<?= tpl_text('Due on %s', ucwords(tpl_strftime('%d %b %Y', $node->data['TS_RESOLVE_BY']))) ?>,
		<?= tpl_text('Amount %d', $node->getAmount()) ?>
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