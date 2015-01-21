<div class="Item">
	<div class="Header">
		<h3><a href="<?= tpl_link($node->getHandler(),'view',$node->nodeId,'design') ?>"><?= $node->toString() ?></a></h3>
		<div class="Info">
			<?= tpl_text('Posted on %s', ucwords(tpl_strftime('%d %B %Y', $node->data['TS_CREATE']))) ?><? 
			if ($node->isPermitted('write')) { ?>;
			<?= tpl_text('%d replies', $node->getAttemptCount()) ?>
			<? } ?>
		</div>
	</div>
</div>