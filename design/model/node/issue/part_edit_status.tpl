<? 
$codes = $node->getDefinedStatusCodes(); 

?>
<input type="hidden" name="data[flags]" value="1" />
<? if ($node->isPermitted('status')) { 

    // Default to waiting if non-active, but only if this is the first load or a form validation error occurred
    if ((!isset($data['status']) || !empty($errors)) && $data['INFO_STATUS'] < synd_node_issue::ACTIVE)
        $data['INFO_STATUS'] = synd_node_issue::PENDING;

?>
	<? if (count($codes) > 4) { ?>
		<select name="data[status]">
			<?= tpl_form_options(array_map('$this->text',SyndLib::array_collect($codes,1)), $data['INFO_STATUS']) ?>
		</select>
	<? } else { ?>
		<? foreach ($codes as $key => $value) { ?>
			<?= tpl_form_radiobutton('data[status]',$data['INFO_STATUS'],$key) ?>
			<label for="data[status][<?= $key ?>]" title="<?= $this->text($value[2]) ?>"><?= $this->text($value[1]) ?></label><br />
		<? } ?>
	<? } ?>
<? } else { ?>
	<?= $this->text($codes[$node->data['INFO_STATUS']][1]) ?>
<? } ?>
