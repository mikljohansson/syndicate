<div class="Article">
	<div class="Header">
		<h1><?= $node->isNew() ? tpl_text('New inventory item') : $node->toString()?></h1>
		<? include tpl_design_path('gui/errors.tpl'); ?>
	</div>
	<? 
	$pane = clone $this;
	$pane->append('tabs', array(
		'uri' => tpl_link_jump($node->getHandler(),'edit',$node->nodeId),
		'text' => tpl_text('Info'),
		'selected' => null == $request[0],
		'template' => array($node,'pane_edit.tpl')));
	
	$pane->append('tabs', array(
		'uri' => tpl_link_jump($node->getHandler(),'edit',$node->nodeId,'files'),
		'text' => tpl_text('Files'),
		'selected' => 'files' == $request[0],
		'template' => array($node,'pane_edit_files.tpl')));

	$pane->display(tpl_design_path('gui/pane/tabbed_form.tpl'),$_data);
	?>

	<p>
		<span title="<?= tpl_text('Accesskey: %s','S') ?>">
			<input accesskey="s" type="submit" name="post" value="<?= $node->isNew()?tpl_text('Add item'):tpl_text('Save') ?>" />
		</span>
		<span title="<?= tpl_text('Accesskey: %s','A') ?>">
			<input accesskey="a" type="button" value="<?= tpl_text('Abort') ?>" onclick="window.location='<?= tpl_uri_return() ?>';" />
		</span>
		<? if (!$node->isNew()) { 
			$parent = $node->getParent(); ?>
		<input class="button" type="button" value="<?= tpl_text('Delete') ?>" onclick="window.location='<?= 
			tpl_view_call($node->getHandler(),'delete',$node->nodeId) ?>';" />
		<? } ?>
	</p>
</div>