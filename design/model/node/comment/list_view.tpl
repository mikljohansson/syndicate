<div class="Item">
	<div class="Info">
		<?= tpl_translate('Posted by %s on %s', 
			$this->fetchnode($node->getCustomer(),'head_view.tpl'), 
			tpl_quote(ucwords(tpl_strftime('%B %d, %Y', $node->data['TS_CREATE'])))) ?>
		<? if ($node->isPermitted('write')) { ?>
			- <a href="<?= tpl_link_call($node->getHandler(),'delete',$node->nodeId) ?>"><?= tpl_text('Delete') ?></a>
		<? } ?>
	</div>
	<div class="Body">
		<?= tpl_html_links($node->data['INFO_BODY'], false, '_blank') ?>
	</div>
</div>
