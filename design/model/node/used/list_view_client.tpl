<? $customer = $node->getCustomer(); ?>
<div class="Item">
	<? if ($node->isActive()) { ?>
		<? $this->render($customer,'contact.tpl') ?>
	<? } else { ?>
		<a href="<?= tpl_link('inventory','view',$customer->nodeId) ?>"><?= $customer->toString() ?></a>
		<? if (null != $customer->getContact()) { ?><span class="Info">(<?= $customer->getContact() ?>)</span><? } ?>
	<? } ?>
	<div class="Info">
		<?= tpl_text('Handed out %s by %s', ucwords(tpl_strftime('%d %b %Y', $node->data['TS_CREATE'])), 
			$this->fetchnode($node->getCreator(),'head_view.tpl')) ?>,
		<? if (!$node->isActive()) { ?>
			<?= tpl_text('Returned %s', ucwords(tpl_strftime('%d %b %Y', $node->data['TS_EXPIRE']))) ?>
		<? } else { ?>
			<?= tpl_text('Not returned') ?>
		<? } ?>
	</div>
</div>