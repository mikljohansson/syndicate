<div class="Item">
	<a href="<?= tpl_link('inventory','view',$node->nodeId) ?>"><?= $node->getSerial() ?> <?= $node->toString() ?></a>
	<div class="Info">
		<? 
		$client = $node->getCustomer();
		if (!$client->isNull()) 
			$location = $this->fetchnode($client,'head_view.tpl');
		else 
			$location = $this->fetchnode($node->getParent(),'head_view.tpl');
		print tpl_text('Current location: %s', $location);
		?>
	</div>
</div>