<input type="hidden" name="data[flags]" value="1" />
<? if (count($codes = $node->getDefinedStatusCodes()) > 4) { ?>
	<select name="data[INFO_STATUS]">
		<?= tpl_form_options(array_map('$this->text',SyndLib::array_collect($codes,1)), $data['INFO_STATUS']) ?>
	</select>
<? } else { ?>
	<? foreach ($codes as $key => $value) { ?>
		<?= tpl_form_radiobutton('data[INFO_STATUS]',$data['INFO_STATUS'],$key) ?>
		<label for="data[INFO_STATUS][<?= $key ?>]" title="<?= $this->text($value[2]) ?>"><?= $this->text($value[1]) ?></label><br />
	<? } ?>
<? } ?>
