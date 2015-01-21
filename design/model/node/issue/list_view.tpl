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
		<?= tpl_text('Resolve by %s', ucwords(tpl_strftime('%d %b %Y', $node->data['TS_RESOLVE_BY']))) ?>
	</div>
	<div class="Abstract">
		<? if (isset($highlight)) { ?>
			<? 
			$data = SyndLib::array_kintersect($node->data, array('INFO_HEAD'=>1,'INFO_BODY'=>1));
			print tpl_def(SyndHTML::getContextSummary($data, $highlight), $node->data['INFO_HEAD']);
			?>
		<? } else { ?>
			<?= tpl_chop(strip_tags($node->data['INFO_BODY']), 300) ?>
		<? } ?>
	</div>
</div>