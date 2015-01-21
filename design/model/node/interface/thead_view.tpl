<table class="Interfaces">
	<thead>
		<tr>
			<th width="10">&nbsp;</th>
			<th><?= tpl_text('IP-address') ?></th>
			<th><?= tpl_text('Hostname') ?></th>
			<th><?= tpl_text('MAC-address') ?></th>
			<th width="1%"><?= tpl_text('Model') ?></th>
			<th width="1%"><?= tpl_text('S/N') ?></th>
			<th><?= tpl_text('Customer') ?></th>
			<th class="Status">&nbsp;</th>
			<? if (empty($hideCheckbox)) { ?>
			<th width="10">
				<? if (isset($collection)) { ?>
				<?= tpl_form_checkbox('collections[]',false,$collection->id()) ?>
				<? } else print '&nbsp;'; ?>
			</th>
			<? } ?>
		</tr>
	</thead>
	<tbody>	
