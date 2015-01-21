<li class="Question">
	<a href="<?= tpl_link('course','view',$node->nodeId) ?>" 
		title="<?= $node->getPageNumber() ?> <?= tpl_value($node->toString()) ?>">
	<?= tpl_chop($node->toString(),20) ?></a>
</li>