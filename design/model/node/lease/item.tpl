<? $inventory = Module::getInstance('inventory'); ?>
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
			<? if (null != $node->data['COSTCENTER_NODE_ID'] || null != $node->data['PROJECT_NODE_ID']) { 
				$costcenter = $node->getCostcenter(); $project = $node->getProject(); ?>
			<th>
				<b><?= tpl_text('Costcenter') ?></b> <?= $costcenter->toString() ?><? 
				if (null != $costcenter->getContact()) { ?> <span class="Info">(<?= $costcenter->getContact() ?>)</span><? } ?>, 
				<b><?= tpl_text('Project') ?></b> <?= $project->toString() ?>
				<? if (null != $project->getContact()) { ?><span class="Info">(<?= $project->getContact() ?>)</span><? } ?>
			</th>
			<? } else { ?>
			<th>&nbsp;</th>
			<? } ?>
			<th>&nbsp;</th>
		</tr>
	</thead>
	<tbody>
		<tr class="odd">
			<td>
				<ul class="Actions">
					<li><a href="<?= tpl_link('inventory','view',$node->nodeId) ?>"><?= tpl_text('Display extended information') ?></a></li>
					<? if ($node->isPermitted('write')) { ?>
						<? if (!$node->isTerminated()) { ?>
						<li><a href="<?= tpl_link_call('inventory','newLeasing',array('lease_node_id'=>$node->nodeId)) ?>"><?= tpl_text('Hand out an item') ?></a></li>
						<li><a href="<?= tpl_link_call('inventory','invoke',$node->nodeId,'terminate') ?>"><?= tpl_text('Terminate this lease') ?></a></li>
						<? } ?>
						<? if (!$node->isTerminated() && null != ($project = $inventory->getRepairProject())) { ?>
						<li><a href="<?= tpl_link_call('inventory','invoke',$node->nodeId,'newIssue') ?>"><?= tpl_text('Create support issue') ?></a></li>
						<li><a href="<?= tpl_link_call('inventory','invoke',$node->nodeId,'newInvoice') ?>"><?= tpl_text('Create general invoice') ?></a></li>
						<li><a href="<?= tpl_link_call('inventory','invoke',$node->nodeId,'newTerminationInvoice') ?>"><?= tpl_text('Create termination invoice') ?></a></li>
						<? } ?>
					<? } ?>
				</ul>

				<? if (null != $node->data['INFO_BODY']) { ?>
				<h4><?= tpl_text('Description') ?></h4>
				<?= tpl_html_format($node->data['INFO_BODY']) ?>
				<? } ?>
			</td>
			<td colspan="2">
				<? if (count($leases = $node->getLeasings())) { ?>
				<h4><?= tpl_text('Leased items') ?></h4>
				<ul class="Enumeration">
					<? foreach ($leases as $lease) { ?>
					<li><? $this->render($lease->getItem(),'head_view.tpl') ?></li>
					<? } ?>
				</ul>
				<? } ?>
				<? if (count($leases = $node->getInactiveLeasings())) { ?>
				<div class="Info"><a href="<?= tpl_link($node->getHandler(),'view',$node->nodeId) ?>"><?= tpl_text('%d previously leased items',count($leases)) ?></a></div>
				<? } ?>

				<? if (count($descriptions = $node->getServiceLevelDescriptions())) { ?>
				<h4><?= tpl_text('Service level descriptions') ?></h4>
				<ul class="Enumeration">
					<? foreach (array_keys($descriptions) as $key) { ?>
					<li><? $this->render($descriptions[$key],'head_view.tpl') ?></li>
					<? } ?>
				</ul>
				<? } ?>
			</td>
		</tr>
	</tbody>
</table>
