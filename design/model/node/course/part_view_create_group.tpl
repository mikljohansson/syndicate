<form action="<?= tpl_view_jump($node->getHandler(),'insert','group') ?>" method="post">
	<? $newGroup = $node->_createGroup($group); ?>

	<input type="hidden" name="node_id" value="<?= $newGroup->nodeId ?>" />

	<div class="indent">
		<?= tpl_text('Creating new subgroup of %s', $group->toString()) ?>
	</div>
	
	<h3><?= tpl_text('Name of group') ?></h3>
	<div class="indent">
		<input type="text" name="data[INFO_HEAD]" />
	</div>

	<input type="submit" name="post" value="<?= tpl_text('Save') ?>" />
	<input type="button" value="<?= tpl_text('Abort') ?>" onclick="window.location='<?= tpl_uri_return() ?>';" />
</form>