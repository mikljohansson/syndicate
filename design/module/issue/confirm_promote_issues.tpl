<form method="post">
	<input type="hidden" name="collections[]" value="<?= $collection->id() ?>" />
	<div class="Dialogue">
		<h1><?= tpl_text('Promote subissues to full issues') ?></h1>
		<p><?= tpl_text('Do you want to promote these subissues to full top-level issues?') ?></p>
		<input type="submit" name="confirm" value="<?= tpl_text('Ok') ?>" />
		<input type="button" value="<?= tpl_text('Cancel') ?>" onclick="window.location='<?= tpl_uri_return() ?>';" />
	</div>
</form>
<? $this->display('model/node/issue/table.tpl', array('list'=>$collection->getFilteredContents(array('synd_node_issue')),'hideCheckbox'=>true)) ?>