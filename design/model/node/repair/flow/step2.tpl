<input type="hidden" name="data[confirm]" value="1" />

<div class="Article">
	<div class="Header">
		<h2><?= tpl_text('Recieve item for repair') ?></h2>
		<div class="Info"><?= tpl_text('Repair that does not require the client to have his/her equipment replaced. For example reinstalling the operating system.') ?></div>
	</div>

	<? if (count($node->getItems())) { ?>
	<div class="Notice">
		<h3><?= tpl_text('Item to recieve for repair') ?></h3>
		<? $this->iterate($node->getItems(),'list_view.tpl') ?>
	</div>
	<? } ?>

	<div class="RequiredField">
		<h3><?= tpl_text('Repair information') ?></h3>
		<? if ($node->data['FLAG_NO_WARRANTY']) { ?>
		<div class="Notice"><?= tpl_text('This repair is not covered by warranty.') ?></div>
		<? } ?>
		<? $this->render($node->getContent(),'full_view.tpl') ?>
	</div>

	<?
	$inventory = Module::getInstance('inventory'); 
	$repairFolder = $inventory->getRepairFolder();

	if (count($node->getItems()) && (null != $repairFolder || null != $terminateFolder)) { ?>
	<div class="OptionalField">
		<h3><?= tpl_text('Folder to transfer items to') ?></h3>
		<div class="Info">
			<?= tpl_text('Default behaviour is not to move the items but if they need to be sent off for repair another folder can be choosen.') ?>
		</div>
		<select name="data[folder]">
			<option value="">&nbsp;</option>
			<option value="<?= $repairFolder->nodeId ?>"><?= $repairFolder->toString() ?></option>
		</select>
	</div>
	<? } ?>

	<div class="OptionalField">
		<?= tpl_form_checkbox('data[receipt]', 1) ?>
			<label for="data[receipt]"><?= tpl_text('Print receipt to give client') ?></label><br />
		<?= tpl_form_checkbox('data[INFO_STATUS]',$data['INFO_STATUS'],synd_node_issue::CLOSED) ?>
			<label for="data[INFO_STATUS]"><?= tpl_text('Mark issue as closed') ?></label><br />
	</div>

	<? include tpl_design_path('gui/errors.tpl'); ?>

	<span title="<?= tpl_text('Accesskey: %s','S') ?>">
		<input accesskey="s" type="submit" name="post" value="<?= tpl_text('Confirm') ?>" />
	</span>
	<input type="button" value="<?= tpl_text('Abort') ?>" onclick="window.location='<?= tpl_uri_return() ?>'" />
</div>