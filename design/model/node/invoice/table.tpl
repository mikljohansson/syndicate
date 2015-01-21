<? 
$this->displayonce('module/issue/context.tpl');
$this->cycle(array('even','odd'));

if (isset($request['partial_uri']))
	$_SERVER['REQUEST_URI'] = $request['partial_uri'];
if (isset($request['partial']) && $request['partial'] == $partial)
	throw new PartialContentException($this->fetchiterator($list, 'trow_view.tpl', $_data));

?>
<table class="Enumeration Issues">
<thead>
	<tr class="<?= tpl_cycle(array('odd','even')) ?>">
		<th class="Actions">&nbsp;</th>
		<th><a href="<?= tpl_sort_uri('invoice','CLIENT_NODE_ID') ?>"><?= tpl_text('Client') ?></a></th>
		<th width="1%"><a href="<?= tpl_sort_uri('invoice','TS_RESOLVE_BY') ?>"><?= tpl_text('Due date') ?></a></th>
		<th width="1%"><a href="<?= tpl_sort_uri('invoice','INFO_AMOUNT_TAXED') ?>"><?= tpl_text('Amount') ?></a></th>
		<th><a href="<?= tpl_sort_uri('invoice','INFO_HEAD') ?>"><?= tpl_text('Title') ?></a></th>
		<? if (empty($hideCheckbox)) { ?>
		<th width="10">
			<? if (isset($collection)) { ?>
			<?= tpl_form_checkbox('collections[]',
				empty($_REQUEST['collections']) && empty($_REQUEST['selection']),
				$collection->id()) ?>
			<? } else print '&nbsp;'; ?>
		</th>
		<? } ?>
	</tr>
</thead>
<? if ($path && $partial) { ?>
<tbody partial="<?= $path ?><?= false===strpos($path,'?')?'?':'&amp;' ?><?= http_build_query(array(
	'partial'		=> $partial,
	'partial_uri'	=> $_SERVER['REQUEST_URI'],
	'stack'			=> $request['stack']),null,'&amp;') ?>">
<? } else { ?>
<tbody>
<? } ?>
	<? foreach ($list as $node) { ?>
	<? $client = $node->getCustomer(); ?>
	<tr class="<?= tpl_cycle() ?> <?= $node->getStatusName() ?><?= $node->isOverdue()?' Overdue':'' ?> <?= $node->getPriorityName() 
		?>" oncontextmenu="return issue_context_menu(this,event,'<?= tpl_view('rpc','json') ?>');" id="<?= $node->id() ?>">
		<td class="Actions"><?
			if ($node->isPermitted('write')) { 
				?><a title="<?= tpl_text('Edit this issue') ?>" href="<?= tpl_link_call($node->getHandler(),'edit',$node->nodeId) ?>"><img border="0" src="<?= tpl_design_uri('image/icon/record.gif') ?>" /></a><? 
			} else print '&nbsp;' ?>
		</td>
		<td class="nowrap"><? $this->render($client,'head_view.tpl') ?></td>
		<td class="Due"><?= date('Y-m-d', $node->data['TS_RESOLVE_BY']) ?></td>
		<td><?= $node->getAmount() ?></td>
		<td width="75%"><a href="<?= tpl_link($node->getHandler(),$node->objectId()) ?>"><?= $node->getTitle() ?></a></td>
		<? if (empty($hideCheckbox)) { ?>
		<td class="OLE" onmouseover="Issues.show(this,<?= count($issues = $node->getChildren()) ? "Array('".implode("','",SyndLib::collect($issues,'nodeId'))."')" : 'null' ?>);" onmouseout="Issues.hide(this);"><input type="checkbox" name="selection[]" value="<?= $node->id() ?>" /></td>
		<? } ?>
	</tr>
	<? } ?>
</tbody>
</table>