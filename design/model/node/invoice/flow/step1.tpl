<? if ($count = $collection->getFilteredCount(array('synd_node_lease'))) { ?>
	<form method="post">
		<input type="hidden" name="prototype" value="<?= $node->nodeId ?>" />
		<input type="hidden" name="collections[]" value="<?= $collection->id() ?>" />

		<div class="Article">
			<div class="Header">
				<h1><?= tpl_text('New invoice') ?></h1>
				<div class="Info"><?= tpl_text('%d leases selected to receive invoice',$count) ?></div>
			</div>
			<? include tpl_design_path('gui/errors.tpl'); ?>
			
			<div class="RequiredField">
				<h3><?= tpl_text('Amount including taxes') ?></h3>
				<input type="text" name="data[INFO_AMOUNT_TAXED]" value="<?= $data['INFO_AMOUNT_TAXED'] ?>" />
			</div>
			<div class="RequiredField">
				<h3><?= tpl_text('Untaxed amount') ?></h3>
				<input type="text" name="data[INFO_AMOUNT_UNTAXED]" value="<?= $data['INFO_AMOUNT_UNTAXED'] ?>" />
			</div>
			<div class="RequiredField">
				<h3><?= tpl_text('Due date') ?></h3>
				<input type="text" name="data[TS_RESOLVE_BY]" value="<?= tpl_date('Y-m-d', $data['TS_RESOLVE_BY']) ?>" /> (YYYY-MM-DD)
			</div>
			<div class="RequiredField">
				<h3><?= tpl_text('Title') ?></h3>
				<input type="text" name="data[INFO_HEAD]" value="<?= $data['INFO_HEAD'] ?>" size="80" />
			</div>
			<? if (count($receipts = $node->getReceiptOptions())) { ?>
			<div class="RequiredField">
				<h3><?= tpl_text('Invoice template') ?><? if (isset($errors['RECEIPT_NODE_ID'])) print '<span style="color:red;">*</span>'; ?></h3>
				<select name="data[RECEIPT_NODE_ID]">
					<? $this->iterate($receipts,'list_view_option.tpl',array('selected'=>$node->getReceiptTemplate())) ?>
				</select>
			</div>
			<? } ?>
			<div class="OptionalField">
				<h3><?= tpl_text('Description') ?></h3>
				<?= tpl_form_textarea('data[content]',$data['content'],array('cols'=>60)) ?>
			</div>

			<input type="submit" name="post" value="<?= tpl_text('Proceed >>>') ?>" />
		</div>
	</form>
<? } else { ?>
	<div class="Warning">
		<h3><?= tpl_text('No leases selected, please make a selection first') ?></h3>
	</div>
<? } ?>