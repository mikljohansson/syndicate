<li class="Question">
	<a href="<?= tpl_link('course','view',$node->nodeId) ?>" 
		title="<?= $node->getPageNumber() ?> <?= tpl_value($node->toString()) ?>">
		<u><?= tpl_chop($node->toString(),20) ?></u></a>
</li>