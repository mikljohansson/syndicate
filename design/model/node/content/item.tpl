<div class="Content">
	<div class="Header">
		<h2><?= $node->toString() ?></h2>
		<? if (null != $node->getDescription()) { ?>
		<h3><?= $node->getDescription() ?></h3>
		<? } ?>
	</div>
</div>