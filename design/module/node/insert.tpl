<form enctype="multipart/form-data" method="post">
	<input type="hidden" name="node_id" value="<?= $node->nodeId ?>" />
	<? $this->render($node,$view,$_data) ?>
</form>