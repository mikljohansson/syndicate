<h1><?= tpl_text('Batch import items') ?></h1>

<form method="post">
	<div class="RequiredField">
		<table class="Vertical">
			<tr class="<?= $this->cycle(array('odd','even')) ?><? if (isset($errors['class'])) print ' InvalidField'; ?>">
				<th><?= $this->text('Folder') ?></th>
				<td>
					<select name="folder">
						<option value="">&nbsp;</option>
						<? $this->iterate(SyndLib::sort(SyndLib::filter($folders,'isPermitted','read')),'option_expand_children.tpl',array('selected'=>$folder)) ?>
					</select>
				</td>
			</tr>
			<tr class="<?= $this->cycle() ?><? if (isset($errors['class'])) print ' InvalidField'; ?>">
				<th><?= $this->text('Category') ?></th>
				<td>
					<select name="class" onchange="this.form.submit();">
						<option value="">&nbsp;</option>
						<? $this->iterate($classes,'option.tpl',array('selected'=>$class)) ?>
					</select>
				</td>
			</tr>
		</table>
	</div>

	<div class="RequiredField">
		<h3><?= $this->text('Please select the fields to import') ?></h3>
		<div class="Info"><?= $this->text('Click field name to remove it') ?></div>
		<? if ($fields) { ?>
			<? foreach ($fields as $key => $field) { ?>
				<input type="hidden" name="fields[]" value="<?= $key ?>" id="field_<?= $key ?>" />
				<a href="#" onclick="field = document.getElementById('field_<?= $key ?>'); field.value = ''; field.form.submit();" title="<?= 
					$this->text('Click to remove this field from import') ?>">&lt;<?= $field ?>&gt;</a>
			<? } ?>
		<? } ?>
		<? if ($fieldOptions) { ?>
		<select name="fields[]" onchange="this.form.submit();">
			<option value="">&nbsp;</option>
			<?= tpl_form_options($fieldOptions) ?>
		</select>
		<? } ?>
	</div>

	<div class="RequiredField">
		<h3><?= $this->text('CSV formatted data') ?></h3>
		<div class="Info"><?= $this->text("Supply comma or tab-separated list containing serial numbers, mac addresses, etc. For example:") ?></div>
		<pre>
 Lenovo	ThinkPad T61	L3A7450	00197E57665C	001A6B38E01C
 Lenovo	ThinkPad T61	L3A7451	00197E576655	001A6B38E021
		</pre>
		<div class="Notice"><?= $this->text('Take care to get the field ordering right and ensure that all data is correct, though input validation is performed it might not guard against all errors. If using Excel, beware of its Fill-Series feature when drag-copying cell contents') ?></div>
		<?= tpl_form_textarea('csv',$request['csv'],array('cols'=>100),40,20,20) ?>
	</div>

	<? include tpl_design_path('gui/errors.tpl'); ?>

	<input type="submit" name="confirm" value="<?= tpl_text('Import &gt;&gt;&gt;') ?>" />
</form>
