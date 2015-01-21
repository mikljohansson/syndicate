		<? include $this->path('synd_node_issue','part_edit_status.tpl'); ?>
		<?= tpl_form_checkbox("data[FLAG_NO_WARRANTY]", $data['FLAG_NO_WARRANTY']) ?> 
			<label for="data[FLAG_NO_WARRANTY]"><?= tpl_text('No warranty') ?></label><br />