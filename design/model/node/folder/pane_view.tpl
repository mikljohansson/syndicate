<? if ($node->isPermitted('write')) { ?>
<ul class="Actions">
	<li><a href="<?= tpl_link_call($node->getHandler(),'invoke',$node->nodeId,'newContent') 
		?>"><?= tpl_text('Create content in folder %s', $node->toString()) ?></a></li>
	<li><a href="<?= tpl_link_call($node->getHandler(),'invoke',$node->nodeId,'newFolder') 
		?>"><?= tpl_text('Create subfolder to %s', $node->toString()) ?></a></li>
	<li><a href="<?= tpl_link_call($node->getHandler(),'edit',$node->nodeId) ?>">
		<?= tpl_text('Edit this folder') ?></a></li>
</ul>
<? } ?>

<?= tpl_gui_table('folder',SyndLib::sort(SyndLib::filter($node->getChildren(),'isPermitted','read'),'toString'),'view.tpl') ?>
