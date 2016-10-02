	<? $customer = $node->getCustomer(); ?>
	<tr class="<?= $this->cycle() ?> <?= $node->getStatusName() ?><?= $node->isOverdue()?' Overdue':'' ?> <?= $node->getPriorityName() 
		?>" oncontextmenu="return issue_context_menu(this,event,'<?= tpl_view('rpc','json') ?>');" id="<?= $node->id() ?>">
		<td class="Actions"><? 
			if ($node->isPermitted('write')) { 
				?><a title="<?= $this->text('Edit this issue') ?>" href="<?= tpl_link_call('issue','edit',$node->nodeId) 
					?>"><img src="<?= tpl_design_uri('image/icon/record.gif') ?>" alt="<?= $this->text('Edit') ?>" /></a><? 
			} else print '&nbsp;' ?>
		</td>
		<td class="nowrap">
			<a href="<?= $this->href('user','summary',$customer->nodeId) ?>"><?= tpl_chop($customer->toString(),25) ?></a>
		</td>
		<td class="Subject" width="75%"><a href="<?= $this->href('issue',$node->objectId()) ?>" title="<?= 
			tpl_attribute($node->getExcerpt()) ?>"><?= $node->getTitleCategories() ?> <?= 
			synd_htmlspecialchars(tpl_chop($node->getTitle(),65)) ?></a></td>
		<td class="Due" title="<?= ucwords(tpl_strftime('%A, %d %B %Y', $node->data['TS_RESOLVE_BY'])) ?>"><?= 
			ucwords(tpl_strftime('%d %b', $node->data['TS_RESOLVE_BY'])) ?></td>
	</tr>
	