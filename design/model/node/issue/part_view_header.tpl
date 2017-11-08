<? 
$this->displayonce('module/issue/context.tpl'); 
$codes = $node->getDefinedStatusCodes(); 
$prios = $node->getDefinedPriorities();

?>
<table class="Vertical Header <?= $node->getStatusName() ?>" oncontextmenu="return issue_context_menu(this,event,'<?= tpl_view('rpc','json') ?>');" id="<?= $node->id() ?>">
	<thead>
		<tr class="<?= $this->cycle(array('odd','even')) ?>">
			<td rowspan="3" class="Status"><img src="<?= tpl_design_uri('image/pixel.gif') ?>" alt="" /></td>
			<th><?= $this->text('Customer') ?></th>
			<td><? $this->render($node->getCustomer(),'contact.tpl',array('extended'=>true)) ?></td>
			<th><?= $this->text('Created') ?></th>
			<td><?= ucwords(tpl_strftime('%a, %d %b %Y %R', $node->data['TS_CREATE'])) ?></td>
			<td rowspan="3" class="Status"><img src="<?= tpl_design_uri('image/pixel.gif') ?>" alt="" /></td>
		</tr>
		<tr class="<?= $this->cycle() ?>">
			<th><?= $this->text('Assigned') ?></th>
			<td><? $this->render($node->getAssigned(),'contact.tpl') ?></td>
			<th><?= $this->text('Due date') ?></th>
			<td><?= ucwords(tpl_strftime('%a, %d %b %Y', $node->getResolveBy())) ?></td>
		</tr>
		<tr class="<?= $this->cycle() ?>">
			<th><?= $this->text('Priority') ?></th>
			<td><?= $this->text($prios[$node->getPriority()]) ?></td>
			<th><?= $this->text('Status') ?></th>
			<td><?= $this->text($codes[$node->data['INFO_STATUS']][1]) ?></td>
		</tr>
	</thead>
</table>
