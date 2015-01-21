<div class="Item">
	<p class="Info"><? 
		print tpl_translate('Posted by %s on %s', 
			$this->fetchnode($node->getCustomer(),'head_view.tpl'), 
			tpl_quote(ucwords(tpl_strftime('%A, %d %b %Y %H:%M', $node->data['TS_ORIGINAL']))));
		if ($node->isPermitted('write')) {
			print '; <a href="'.tpl_view_call($node->getHandler(),'delete',$node->nodeId).'">';
			print tpl_text('Delete');
			print '</a>';
		} 
	?></p>
	<p class="Body"><?= tpl_html_links($node->getBody()) ?></p>
</div>
