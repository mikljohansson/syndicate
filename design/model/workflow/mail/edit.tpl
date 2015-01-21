<? $base = 'mplex[mplex;'.$workflow->id().';'.$node->id().'/setSelectedOption/]'; ?>
<?= tpl_form_checkbox("{$base}[customer]",$node->isCustomerSelected()) ?>
	<?= $this->text('Customer') ?>
<?= tpl_form_checkbox("{$base}[assigned]",$node->isAssignedSelected()) ?>
	<?= $this->text('Assigned') ?>

<b><?= $this->text('Cc') ?>:</b>
<?= tpl_form_input("{$base}[cc]",$node->getCc()) ?>