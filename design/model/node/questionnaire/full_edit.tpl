<div class="Article" style="width:500px;">
	<?= tpl_text('Title') ?>
	<div class="indent">
		<input type="text" name="data[INFO_HEAD]" value="<?= tpl_value($node->data['INFO_HEAD']) ?>" style="width:500px;" maxlength="255" />
	</div>

	<?= tpl_text('Text') ?>
	<div class="indent">
	<? $this->render($node->data['INFO_BODY'],'full_edit.tpl',array(
		'id'    => 'data[INFO_BODY]', 
		'style' => array('width'=>'500px', 'height'=>'100px'))) ?>
	</div>

	<br />
	<div class="indent">
		<input type="submit" class="button" name="post" value="<?= tpl_text('Save') ?>" />
		<input type="button" class="button" value="<?= tpl_text('Abort') ?>"  onclick="window.location='<?= tpl_uri_return() ?>';" />
		<? if (!$node->isNew()) { ?>
		<? $parent = $node->getParent(); ?> 
		<input class="button" type="button" value="<?= tpl_text('Delete') ?>" 
			onclick="window.location='<?= tpl_view_call($node->getHandler(),'delete',$node->nodeId,
				tpl_view($parent->getHandler(),'view',$parent->nodeId)) ?>';" />
		<? } ?>
	</div>
</div>