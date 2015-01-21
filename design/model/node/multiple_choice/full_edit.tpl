<div class="Article">
	<? include tpl_design_path('gui/errors.tpl'); ?>
	<div class="RequiredField">
		<h3><?= tpl_text('Question') ?></h3>
		<?= tpl_form_textarea('data[INFO_QUESTION]',$data['INFO_QUESTION'],array('cols'=>72)) ?>
	</div>

	<div class="RequiredField">
		<h3><?= tpl_text('Appearance/Layout') ?></h3>
		<? $attrib = array('onclick' => 'this.form.submit()'); ?>
		<?= tpl_form_radiobutton('data[INFO_LAYOUT]',$data['INFO_LAYOUT'],'radio',null,$attrib) ?>
			<label for="data[INFO_LAYOUT][radio]"><?= tpl_text('Single answer (radiobuttons)') ?></label>
			<div class="Info" style="margin-left:2.5em;"><?= tpl_text('Displays the options below the question allowing just one option to be selected.') ?></div>
		
		<?= tpl_form_radiobutton('data[INFO_LAYOUT]',$data['INFO_LAYOUT'],'checkbox',null,$attrib) ?>
			<label for="data[INFO_LAYOUT][checkbox]"><?= tpl_text('Multiple answers (checkboxes)') ?></label><br />
			<div class="Info" style="margin-left:2.5em;"><?= tpl_text('Displays the options below the question allowing for multiple options to be selected.') ?></div>

		<?= tpl_form_radiobutton('data[INFO_LAYOUT]',$data['INFO_LAYOUT'],'inline_select',null,$attrib) ?>
			<label for="data[INFO_LAYOUT][inline_select]"><?= tpl_text('Drop-down inside of question') ?></label><br />
			<div class="Info" style="margin-left:2.5em;"><?= tpl_text('A drop-down menu inside the question, use &lt;10&gt; to indicate the location of the meny.') ?></div>

		<?= tpl_form_radiobutton('data[INFO_LAYOUT]',$data['INFO_LAYOUT'],'inline_radio',null,$attrib) ?>
			<label for="data[INFO_LAYOUT][inline_radio]"><?= tpl_text('Single answer inside of question') ?></label><br />
			<div class="Info" style="margin-left:2.5em;"><?= tpl_text('A number of radiobuttons inside the question, use &lt;(1) where&gt; to create a button.') ?></div>

		<?= tpl_form_radiobutton('data[INFO_LAYOUT]',$data['INFO_LAYOUT'],'inline_checkbox',null,$attrib) ?>
			<label for="data[INFO_LAYOUT][inline_checkbox]"><?= tpl_text('Multiple answers inside of question') ?></label><br />
			<div class="Info" style="margin-left:2.5em;"><?= tpl_text('A number of checkboxes inside the question, use &lt;(1) where&gt; to create one.') ?></div>
	</div>
	
	<? if (!$node->hasInlineOptions()) { ?>
	<div class="RequiredField">
		<h3><?= tpl_text('Options (one per line)') ?></h3>
		<?= tpl_form_textarea('data[options]',$data['options'],array('cols'=>72)) ?>
	</div>
	<? } ?>

	<div class="OptionalField">
		<h3><?= tpl_text('Correct answers (one per line)') ?></h3>
		<?= tpl_form_textarea('data[INFO_CORRECT_ANSWER]',$data['INFO_CORRECT_ANSWER'],array('cols'=>72)) ?>
	</div>

	<div class="OptionalField">
		<h3><?= tpl_text('Explanations (shown together with the answer)') ?></h3>
		<table>
			<tr>
				<td>
					<h4><?= tpl_text('For correct answers') ?></h4>
					<?= tpl_form_textarea('data[INFO_CORRECT_EXPLANATION]',
						$data['INFO_CORRECT_EXPLANATION'],array('cols'=>33)) ?>
				</td>
				<td>
					<h4><?= tpl_text('For incorrect answers') ?></h4>
					<?= tpl_form_textarea('data[INFO_INCORRECT_EXPLANATION]',
						$data['INFO_INCORRECT_EXPLANATION'],array('cols'=>33)) ?>
				</td>
			</tr>
		</table>
	</div>

	<div class="OptionalField">
		<? $this->render($node,'part_edit_flags.tpl',$_data) ?>
	</div>

	<span title="<?= tpl_text('Accesskey: %s','S') ?>">
		<input accesskey="s" type="submit" name="post" value="<?= tpl_text('Save') ?>" />
	</span>
	<input type="button" value="<?= tpl_text('Abort') ?>"  onclick="window.location='<?= tpl_uri_return() ?>';" />
	<? if (!$node->isNew()) { ?>
		<? $parent = $node->getParent(); ?>
		<input class="button" type="button" value="<?= tpl_text('Delete') ?>" 
			onclick="window.location='<?= tpl_view_call($node->getHandler(),'delete',$node->nodeId,
				tpl_view($parent->getHandler(),'view',$parent->nodeId)) ?>';" />
	<? } ?>
</div>