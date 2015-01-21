<div class="Item">
	<div class="Info"><?= tpl_text('Repaired') ?></div>
	<div style="margin-left:5px;">
		<? $item = $node->getItem(); ?>
		<? $this->render($item,'head_view.tpl') ?> <span class="Info">(<? $this->render($item->getParent(),'head_view.tpl') ?>)</span><br />
	</div>

	<? $replacement = $node->getReplacement(); ?>
	<? if (!$replacement->isNull()) { ?>
		<div class="Info"><?= tpl_text('Replacement') ?></div>
		<div style="margin-left:5px;">
			<? $this->render($replacement,'head_view.tpl') ?> <span class="Info">(<? $this->render($replacement->getParent(),'head_view.tpl') ?>)</span>
		</div>
	<? } else { ?>
		<div class="Info" style="margin-left:5px;">
			<em><?= tpl_text('No replacement selected') ?></em>
		</div>
	<? } ?>
</div>