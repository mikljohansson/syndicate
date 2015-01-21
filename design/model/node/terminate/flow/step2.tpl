<?= tpl_load_script(tpl_design_uri('js/form.js')) ?>
<input type="hidden" name="data[confirm]" value="1" />

<div class="Article">
	<div class="Header">
		<h2><?= tpl_text('Confirm termination of lease') ?></h2>
		<? $this->render($node->getCustomer(),'list_view.tpl') ?>
	</div>

	<? if (count($node->getItems())) { ?>
	<div class="Notice">
		<h2><?= tpl_text('Items to be returned by client') ?></h2>
		<? $this->iterate($node->getItems(),'list_view.tpl') ?>
	</div>
	<? } ?>

	<div class="RequiredField">
		<h3><?= tpl_text('Notes') ?></h3>
		<div class="Info"><?= tpl_text('Supply information on the condition of returned items, any repairs that needs to be done or if items are missing. If everything checks out ok you can mark the issue as closed, otherwise leave it open to be followed up.') ?></div>
		<?= tpl_form_textarea('data[content]',$request['data']['content'],
			array('cols'=>53,'match'=>'\w+','message'=>tpl_text('Please provide a description'))) ?>
	</div>

	<?
	$inventory = Module::getInstance('inventory'); 
	$repairFolder = $inventory->getRepairFolder();
	$terminateFolder = $inventory->getTerminateFolder();
	$soldFolder = $inventory->getSoldFolder();

	if (count($node->getItems()) && (null != $repairFolder || null != $terminateFolder)) { ?>
	<div class="OptionalField">
		<h3><?= tpl_text('Folder to transfer items to') ?></h3>
		<div class="Info"><?= tpl_text('Defaults to the returned items folder, but can be changed when the equipment needs to be repaired or is to be sold.') ?></div>
		<select name="data[folder]" onchange="document.getElementById('data[INFO_STATUS]').checked = (1!=this.selectedIndex);">
			<option value="<?= $terminateFolder->nodeId ?>"><?= $terminateFolder->toString() ?></option>
			<option value="<?= $repairFolder->nodeId ?>"><?= $repairFolder->toString() ?></option>
			<? if (null != $soldFolder) { ?>
			<option value="<?= $soldFolder->nodeId ?>"><?= $soldFolder->toString() ?></option>
			<? } ?>
		</select>
	</div>
	<? } ?>

	<div class="OptionalField">
		<?= tpl_form_checkbox('data[receipt]',true) ?>
			<label for="data[receipt]"><?= tpl_text('Print receipt to give client') ?></label><br />
		<?= tpl_form_checkbox('data[INFO_STATUS]',true,synd_node_issue::CLOSED) ?>
			<label for="data[INFO_STATUS]"><?= tpl_text('Mark issue as closed') ?></label><br />
		<?= tpl_form_checkbox('data[FLAG_NO_WARRANTY]', $data['FLAG_NO_WARRANTY']) ?>
			<label for="data[FLAG_NO_WARRANTY]"><?= tpl_text('Non warranty issue (repairs are needed or something is missing)') ?></label>
	</div>

	<? include tpl_design_path('gui/errors.tpl'); ?>

	<br />
	<span title="<?= tpl_text('Accesskey: %s','S') ?>">
		<input accesskey="s" type="submit" name="post" value="<?= tpl_text('Confirm') ?>" />
	</span>
	<input type="button" value="<?= tpl_text('Abort') ?>" onclick="window.location='<?= tpl_uri_return() ?>'" />
</div>
