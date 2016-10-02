	<? $customer = $node->getCustomer(); $assigned = $node->getAssigned(); ?>
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
		<td class="nowrap">
			<a href="<?= $this->href('user','summary',$assigned->nodeId) ?>"><?= tpl_chop($assigned->toString(),25) ?></a>
		</td>
		<td class="Subject" width="75%"><a href="<?= $this->href('issue',$node->objectId()) ?>" title="<?= 
			tpl_attribute($node->getExcerpt()) ?>"><?= $node->getTitleCategories() ?> <?= 
			synd_htmlspecialchars(tpl_chop($node->getTitle(),65)) ?></a></td>
		<td><? $this->render($node->getParent(),'code.tpl') ?></td>
		<td class="Created" title="<?= ucwords(tpl_strftime('%A, %d %B %Y %H:%M', $node->data['TS_CREATE'])) ?>"><?= 
			ucwords(tpl_strftime('%Y-%m-%d', $node->data['TS_CREATE'])) ?></td>
		<td class="Updated" title="<?= ucwords(tpl_strftime('%A, %d %B %Y %H:%M', $node->data['TS_UPDATE'])) ?>"><?= 
			ucwords(tpl_strftime('%d %b', $node->data['TS_UPDATE'])) ?></td>
		<td class="Due" title="<?= ucwords(tpl_strftime('%A, %d %B %Y', $node->data['TS_RESOLVE_BY'])) ?>"><?= 
			ucwords(tpl_strftime('%d %b', $node->data['TS_RESOLVE_BY'])) ?></td>
		<td class="Status"><div><?= tpl_default($node->getOpenCount(),'<img src="'.tpl_design_uri('image/pixel.gif').'" alt="" />') ?></div></td>
		<? if (empty($hideCheckbox)) { ?>
		<td class="OLE" onmouseover="Issues.show(this,<?= count($issues = $node->getChildren()) ? "Array('".implode("','",SyndLib::collect($issues,'nodeId'))."')" : 'null' ?>);" onmouseout="Issues.hide(this);"><input type="checkbox" name="selection[]" value="<?= $node->id() ?>" /></td>
		<? } ?>
	</tr>
	