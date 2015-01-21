<? $file = $node->getFile(); ?>
<div class="Item">
	<div class="Header">
		<h4><a href="<?= tpl_link($node->getHandler(),'view',$node->nodeId) ?>"><?= $node->toString() ?></a></h4>
		<div class="Info">
			<?= tpl_text('Posted by %s on %s', $this->fetchnode($node->getCustomer(),'head_view.tpl'), 
				ucwords(tpl_strftime('%A, %d %B %Y', $node->data['TS_CREATE']))) ?>; 
			<?= ceil($file->getSize()/1024) ?>Kb<? 
			if ($node->isPermitted('write')) { ?>;
			<a href="<?= tpl_link_call('node','edit',$node->nodeId) ?>"><?= tpl_text('Edit this file') ?></a>
			<? } ?>
		</div>
	</div>
</div>