<div class="Item">
	<? $this->render($node,'head_view.tpl') ?>
	<div class="Info">
		<? $this->render($node,'list_view_duration.tpl') ?>
	</div>
	<? if (null != $node->getDescription()) { ?>
	<div class="Abstract">
		<?= $node->getDescription() ?>
	</div>
	<? } ?>
</div>