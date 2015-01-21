<div class="Item" style="margin-top:20px; margin-bottom:20px;">
	<div class="Header"><?= $node->toString() ?></div>
	<div class="Abstract">
		<? $body = $node->getBody(); ?>
		<?= $body->toString() ?>
	</div>
	<div>
		<? $this->iterate($node->getChildren(),'item_statistics.tpl',$_data); ?>
	</div>
</div>
<hr />
