	<? foreach ($node->getActivities() as $key => $activity) { ?>
	<div class="Optional">
		<label><?= $activity ?></label>
		<a class="Action" href="<?= tpl_link_call('mplex;'.$workflow->id().';'.$node->id(),'removeActivity',$key) ?>"><img src="<?= tpl_design_uri('image/icon/trash.gif') ?>" width="16" height="16" alt="<?= tpl_text('Remove') ?>" /></a>
		<? $this->render($activity,'edit.tpl') ?>
	</div>
	<? } ?>

	<div class="Optional<?= isset($errors['action'])?' Invalid':'' ?>">
		<select name="mplex[mplex;<?= $workflow->id() ?>;<?= $node->id() ?>/addActivity/][activity]">
			<option value="">&nbsp;</option>
			<?= tpl_form_options($node->getActivityOptions($workflow)) ?>
		</select>
		<input type="submit" value="<?= $this->text('Add') ?>" />
	</div>
