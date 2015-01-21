<input type="hidden" name="data[confirm]" value="1" />

<div class="Article">
	<div class="Header">
		<h2><?= tpl_text('Replacement of faulty item') ?></h2>
		<div class="Info"><?= tpl_text('Extended repair that requires equipment to be shipped off to be repaired. The client will in most cases have her items replaced.') ?></div>
	</div>

	<? if (count($node->getItems())) { ?>
	<div class="Notice">
		<h3><?= tpl_text('Item to recieve for repair') ?></h3>
		<? 
		$inventory = Module::getInstance('inventory'); 
		if (null != $inventory->getRepairFolder()) { ?>
		<div class="Info"><?= tpl_text('Will be transferred to %s after the replacement.', $this->fetchnode($inventory->getRepairFolder(),'head_view.tpl')) ?></div>
		<? } ?>
		<? $this->iterate($node->getItems(),'list_view.tpl') ?>
	</div>
	<? } ?>

	<? if (count($node->getReplacements())) { ?>
	<div class="Notice">
		<h3><?= tpl_text('Replacement item to give client') ?></h3>
		<div class="Info"><?= tpl_text('Will be transferred to %s after the replacement.', $this->fetchnode($node->getCustomer(),'head_view.tpl')) ?></div>
		<? $this->iterate($node->getReplacements(),'list_view.tpl') ?>
	</div>
	<? } ?>

	<div class="RequiredField">
		<h3><?= tpl_text('Repair information') ?></h3>
		<? if ($node->data['FLAG_NO_WARRANTY']) { ?>
		<div class="Notice"><?= tpl_text('This repair is not covered by warranty.') ?></div>
		<? } ?>
		<? $this->render($node->getContent(),'full_view.tpl') ?>
	</div>
	
	<div class="OptionalField">
		<?= tpl_form_checkbox('data[receipt]', 1) ?>
			<label for="data[receipt]"><?= tpl_text('Print receipt to give client') ?></label><br />
	</div>

	<? include tpl_design_path('gui/errors.tpl'); ?>

	<span title="<?= tpl_text('Accesskey: %s','S') ?>">
		<input accesskey="s" type="submit" name="post" value="<?= tpl_text('Perform replacement') ?>" />
	</span>
	<input type="button" value="<?= tpl_text('Abort') ?>" onclick="window.location='<?= tpl_uri_return() ?>'" />
</div>
