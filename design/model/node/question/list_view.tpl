<div class="Item">
	<div class="Info"><? $this->render($node->getParent(),'head_view.tpl') ?></div>
	<div class="Abstract">
		<? if (isset($highlight)) { ?>
			<? 
			require_once 'core/lib/SyndHTML.class.inc';
			print tpl_def(SyndHTML::getContextSummary($node->toString(), $highlight), $node->toString());
			?>
		<? } else { ?>
			<?= $node->toString() ?>
		<? } ?>
	</div>
</div>