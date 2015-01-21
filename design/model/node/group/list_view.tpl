<div class="Item">
	<a href="<?= tpl_link($node->getHandler(),'view',$node->nodeId) ?>"><?= $node->toString() ?></a>
	<? if ($node->data['INFO_DESC']) { ?>
	<div class="Info indent"><?= tpl_text($node->data['INFO_DESC']) ?></div>
	<? } ?>
</div>