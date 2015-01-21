<form method="post">
	<input type="hidden" name="prototype" value="<?= $node->nodeId ?>" />
	<? $this->render($node,'edit_prototype.tpl') ?>
</form>