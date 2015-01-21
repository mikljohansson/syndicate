<? $parent = $node->getParent(); $client = $node->getCustomer(); ?>
<div class="Info">
	<a href="<?= tpl_link($node->getHandler(),'view',$parent->nodeId,array('attempt'=>$node->nodeId)) ?>"><?
	print tpl_text('Created on %s', ucwords(tpl_strftime('%A, %d %b %Y',$node->data['TS_CREATE'])));
	if (!$client->isNull())
		print tpl_text('; by %s', $client->toString()); ?></a>
</div>