<input type="text" name="mplex[mplex;<?= $workflow->id() ?>;<?= $node->id() ?>/setNumber/][number]" value="<?= $this->quote($node->getNumber()) ?>" />
<small><?= $this->text('Please specify the numeric id of the issue to use as a template, for example 12345') ?></small>
<fieldset class="WorkflowSequence">
	<? $this->render($node,'edit_activities.tpl'); ?>
</fieldset>