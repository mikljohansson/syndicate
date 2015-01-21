<div class="Item">
	<h3><a href="<?= tpl_link_call($node->getHandler(),'edit',$node->nodeId) ?>"><?= $node->toString() ?></a></h3>
	<div class="Info"><?= tpl_text('%s items', $node->getItemCount()) ?></div>
</div>