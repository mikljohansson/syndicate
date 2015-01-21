<fieldset class="WorkflowSequence">
	<? foreach ($node->getActivities() as $activity) { ?>
	<div class="WorkflowActivity">
		<label><?= $activity ?></label>
		<? $this->render($activity,'view.tpl') ?>
	</div>
	<? } ?>
</fieldset>