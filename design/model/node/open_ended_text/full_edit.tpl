<div class="Article">
	<? include tpl_design_path('gui/errors.tpl'); ?>
	<div class="RequiredField">
		<h3><?= tpl_text('Question') ?></h3>
		<?= tpl_form_textarea('data[INFO_QUESTION]',$data['INFO_QUESTION'],array('cols'=>72)) ?>
	</div>

	<div class="RequiredField">
		<h3><?= tpl_text('Appearance/Layout') ?></h3>
		<?= tpl_form_radiobutton('data[INFO_LAYOUT]',$data['INFO_LAYOUT'],'input') ?>
			<label for="data[INFO_LAYOUT][input]"><?= tpl_text('Single line textfield') ?></label><br />
			<div class="Info" style="margin-left:2.5em;"><?= tpl_text('Displays a single line textfield below the qestion.') ?></div>

		<?= tpl_form_radiobutton('data[INFO_LAYOUT]',$data['INFO_LAYOUT'],'textarea') ?>
			<label for="data[INFO_LAYOUT][textarea]"><?= tpl_text('Multiple line textarea') ?></label><br />
			<div class="Info" style="margin-left:2.5em;"><?= tpl_text('Displays a multiple line textarea below the question.') ?></div>

		<?= tpl_form_radiobutton('data[INFO_LAYOUT]',$data['INFO_LAYOUT'],'rearrange') ?>
			<label for="data[INFO_LAYOUT][rearrange]"><?= tpl_text('Rearrange text question (pre-filled-in textarea)') ?></label><br />
			<div class="Info" style="margin-left:2.5em;"><?= tpl_text('Displays a textarea with the question inside which can be edited and submitted.') ?></div>

		<?= tpl_form_radiobutton('data[INFO_LAYOUT]',$data['INFO_LAYOUT'],'inline_input') ?>
			<label for="data[INFO_LAYOUT][inline_input]"><?= tpl_text('Textfield inside question (inline input)') ?></label><br />
			<div class="Info" style="margin-left:2.5em;"><?= tpl_text('A textfield inside the question. Use &lt;xx&gt; (xx is width of field, 1-100) inside question.') ?></div>
	</div>

	<div class="OptionalField">
		<h3><?= tpl_text('Correct answer') ?></h3>
		<?= tpl_form_textarea('data[INFO_CORRECT_ANSWER]',$data['INFO_CORRECT_ANSWER'],array('cols'=>72)) ?>
		<div class="Info"><?= tpl_text("Multiple correct answers (synonyms and other alternate answers) can be specified by writing something like <em>'The &lt;machine/engine/motor&gt; is loud'</em> in the <em>Correct answer</em> field.") ?></div>
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
		<?= tpl_form_checkbox('data[FLAG_CASE_SENSITIVE]', $node->data['FLAG_CASE_SENSITIVE']) ?>
			<?= tpl_text('Case sensitive answer') ?>
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