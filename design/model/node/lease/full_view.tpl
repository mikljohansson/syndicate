<? 
$inventory = Module::getInstance('inventory'); 
SyndLib::attachHook('breadcrumbs', array($node, '_callback_breadcrumbs'));
?>
<table class="Block">
	<thead>
		<tr>
			<th style="width:49%;"><b><?= tpl_text('Created') ?></b> <?= ucwords(tpl_strftime('%A, %d %B %Y', $node->data['TS_CREATE'])) ?></th>
			<? $receipt = $node->getReceiptTemplate(); if (!$receipt->isNull()) { ?>
			<th style="width:49%;"><b><?= tpl_text('Receipt template') ?></b> <? $this->render($receipt,'head_view.tpl') ?></th>
			<? } else { ?>
			<th>&nbsp;</th>
			<? } ?>
			<th class="OLE">
				<? if ($node->isPermitted('write')) { ?>
				<a href="<?= tpl_link_call($node->getHandler(),'invoke',$node->nodeId,'printReceipt') ?>"><img src="<?= tpl_design_uri('image/icon/16x16/print.gif') 
					?>" alt="<?= tpl_text('Print') ?>" title="<?= tpl_text('Print receipt on selected printer') ?>" /></a>
				<a href="<?= tpl_link_call('inventory','edit',$node->nodeId) ?>"><img src="<?= tpl_design_uri('image/icon/edit.gif') 
					?>" alt="<?= tpl_text('Edit') ?>" title="<?= tpl_text('Edit this lease') ?>" /></a>
				<? } ?>
				<input type="checkbox" name="selection[]" value="<?= $node->id() ?>" />
			</th>
		</tr>
		<tr>
			<? if ($node->isTerminated()) { ?>
			<th><b><?= tpl_text('Terminated') ?></b> <?= ucwords(tpl_strftime('%d %b %Y', $node->data['TS_TERMINATED'])) ?></th>
			<? } else if (null != $node->data['TS_EXPIRE']) { ?>
			<th><b><?= tpl_text('Expires') ?></b> <?= ucwords(tpl_strftime('%A, %d %B %Y', $node->data['TS_EXPIRE'])) ?></th>
			<? } else { ?>
			<th><b><?= tpl_text('No expiry date set') ?></b></th>
			<? } ?>
			<? if (null != $node->data['COSTCENTER_NODE_ID']) { $costcenter = $node->getCostcenter(); ?>
			<th>
				<b><?= tpl_text('Costcenter') ?></b> <?= $costcenter->toString() ?>
				<? if (null != $costcenter->getContact()) { ?><span class="Info"><?= $costcenter->getContact() ?></span><? } ?>
			</th>
			<? } else { ?>
			<th>&nbsp;</th>
			<? } ?>
			<th>&nbsp;</th>
		</tr>
	</thead>
	<? if (null != $node->data['INFO_BODY']) { ?>
	<tbody>
		<tr class="<?= tpl_cycle(array('odd','even')) ?>">
			<td colspan="3"><?= tpl_html_format($node->data['INFO_BODY']) ?></td>
		</tr>
	</tbody>
	<? } ?>
</table>

<? if ($node->isPermitted('write')) { ?>
<ul class="Actions">
	<? if (!$node->isTerminated()) { ?>
	<li><a href="<?= tpl_link_call('inventory','newLeasing',array('lease_node_id'=>$node->nodeId)) ?>"><?= tpl_text('Hand out an item') ?></a></li>
	<? } ?>
	<? if (!$node->isTerminated() && null != ($project = $inventory->getRepairProject())) { ?>
	<li><a href="<?= tpl_link_call('inventory','invoke',$node->nodeId,'newIssue') ?>"><?= tpl_text('Create support issue') ?></a></li>
	<? } ?>
</ul>
<? } ?>

<? if (count($descriptions = $node->getServiceLevelDescriptions())) { ?>
<h3><?= tpl_text('Service level descriptions') ?></h3>
<?= tpl_gui_table('sld',$descriptions,'view.tpl') ?>
<? } ?>

<? if (count($leasings = $node->getLeasings())) { ?>
<h3><?= tpl_text('Leased items') ?></h3>
<? $this->display('model/node/used/table.tpl',array('list'=>$leasings)) ?>
<? } ?>

<? if (count($leasings = $node->getInactiveLeasings())) { ?>
<h3><?= tpl_text('Previously leased items') ?></h3>
<? $this->display('model/node/used/table.tpl',array('list'=>$leasings)) ?>
<? } ?>
