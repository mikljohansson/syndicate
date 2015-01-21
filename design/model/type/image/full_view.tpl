<div style="margin-bottom:10px;">
	<a href="<?= $node->uri() ?>" title="<?= $node->toString() ?> (<?= $node->getWidth() ?>x<?= $node->getHeight() ?>px, <?= round($node->getSize()/1000,1) ?>Kb)">
	<img src="<?= $node->getBoxedUri(800,600) ?>" border="0" class="border" />
	</a>
</div>