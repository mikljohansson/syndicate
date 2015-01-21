<? 
$this->displayonce('module/issue/context.tpl'); 
$codes = $node->getDefinedStatusCodes(); 
$prios = $node->getDefinedPriorities();

?>
<table class="Vertical Header" oncontextmenu="return issue_context_menu(this,event,'<?= tpl_view('rpc','json') ?>');" id="<?= $node->id() ?>">
	<thead>
		<tr class="<?= $this->cycle(array('odd','even')) ?>">
			<th><?= $this->text('Customer') ?></th>
			<td><? $this->render($node->getCustomer(),'contact.tpl') ?></td>
			<th><?= $this->text('Created') ?></th>
			<td><?= ucwords(tpl_strftime('%a, %d %b %Y %R', $node->data['TS_CREATE'])) ?></td>
		</tr>
		<tr class="<?= $this->cycle() ?>">
			<th><?= $this->text('Assigned') ?></th>
			<td><? $this->render($node->getAssigned(),'contact.tpl') ?></td>
			<th><?= $this->text('Due date') ?></th>
			<td><?= ucwords(tpl_strftime('%a, %d %b %Y', $node->getResolveBy())) ?></td>
		</tr>
		<tr class="<?= $this->cycle() ?>">
			<th><?= $this->text('Paid') ?></th>
			<td>
				<? if ($node->isPaid()) { ?>
					<?= $this->text('Yes, %s', tpl_strftime('%Y-%m-%d', $node->data['TS_PAID'])) ?>
				<? } else { ?>
					<?= $this->text('No') ?>
				<? } ?>
			</td>
			<th><?= $this->text('Status') ?></th>
			<td><?= $this->text($codes[$node->data['INFO_STATUS']][1]) ?></td>
		</tr>
		<tr class="<?= $this->cycle() ?>">
			<th><?= $this->text('Amount') ?></th>
			<td>
				<?= $this->quote($node->getAmount()) ?>
				<span class="Info">(<?= $this->text('%d without taxes',$node->getUntaxedAmount()) ?>)</span>
			</td>
			<th><?= $this->text('Invoice number') ?></th>
			<td><?= $this->quote($node->getInvoiceNumber()) ?></td>
		</tr>
	</thead>
</table>
