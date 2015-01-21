<? include $this->path('synd_node_issue','part_edit_header.tpl'); ?>
<table class="Vertical">
	<? if (count($receipts = $node->getReceiptOptions())) { ?>
	<tr class="<?= $this->cycle() ?>">
		<th><?= tpl_text('Receipt') ?></th>
		<td>
			<select tabindex="<?= $this->sequence() ?>" name="data[RECEIPT_NODE_ID]">
				<? $this->iterate($receipts,'list_view_option.tpl',array('selected'=>$node->getReceiptTemplate())) ?>
			</select>
		</td>
	</tr>
	<? } ?>
	<tr class="<?= $this->cycle() ?>">
		<th><?= tpl_text('Amount') ?></th>
		<td>
			<input tabindex="<?= $this->sequence() ?>" type="text" name="data[INFO_AMOUNT_TAXED]" value="<?= tpl_attribute($data['INFO_AMOUNT_TAXED']) ?>" style="width:6em;" /> <?= tpl_text('taxed') ?>
			<input tabindex="<?= $this->sequence() ?>" type="text" name="data[INFO_AMOUNT_UNTAXED]" value="<?= tpl_attribute($data['INFO_AMOUNT_UNTAXED']) ?>" style="width:6em;" /> <?= tpl_text('untaxed') ?>
		</td>
	</tr>
	<tr class="<?= $this->cycle() ?>">
		<th><?= tpl_text('Paid') ?></th>
		<td>
			<?= tpl_form_checkbox('data[paid]',$data['paid'],1,null,array('tabindex'=>$this->sequence())) ?>
			<label for="data[paid]"><?= tpl_text('Invoice paid') ?></label><br />
		</td>
	</tr>
</table>