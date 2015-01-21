<div class="FlowingForm">
	<fieldset>
		<legend><?= $this->text('Workflow') ?></legend>
		<div class="Required">
			<label><?= $this->text('Project') ?></label>
			<p><? $this->render($node->getParent(),'head_view.tpl') ?></p>
		</div>
		<div class="Required">
			<label><?= $this->text('Name') ?></label>
			<p><?= $this->quote($node->toString()) ?></p>
		</div>
		<div class="Optional">
			<label><?= $this->text('Description') ?></label>
			<p><?= $this->format($node->data['INFO_DESC']) ?></p>
		</div>
		<div class="Optional<?= isset($errors['FLAG_CONTEXT_MENU'])||isset($errors['FLAG_SIDEBAR_MENU'])?' Invalid':'' ?>">
			<label><?= $this->text('Display') ?></label>
			<p>
				<? if ($node->data['FLAG_CONTEXT_MENU']) { ?>
				<?= $this->text('Display in context menu') ?><br />
				<? } ?>
				<? if ($node->data['FLAG_SIDEBAR_MENU']) { ?>
				<?= $this->text('Display in sidebar') ?><br />
				<? } ?>
			</p>
		</div>
		<div class="Optional">
			<label><?= $this->text('Accesskey') ?></label>
			<? if (strlen($node->data['INFO_ACCESSKEY'])) { ?>
			<em>Alt+Shift+<?= $this->quote($node->data['INFO_ACCESSKEY']) ?></em> or <em>Alt+<?= $this->quote($node->data['INFO_ACCESSKEY']) ?></em>
			<? } ?>
		</div>
	</fieldset>

	<? if (count($events = $node->getEvents())) { ?>
	<fieldset>
		<legend><?= $this->text('Events') ?></legend>
		<div class="Information">
			<h4><?= $this->text('Events') ?></h4>
			<p><?= $this->text('Events that should trigger this workflow') ?></p>
		</div>
		<div class="Optional">
			<? foreach ($node->getEventOptions() as $event) { ?>
				<? if (in_array(get_class($event),$node->getEvents())) { ?>
				<p><?= $this->text($event->__toString()) ?></p><br />
				<? } ?>
			<? } ?>
		</div>
	</fieldset>
	<? } ?>
	
	<fieldset>
		<legend><?= $this->text('Actions') ?></legend>
		<? $this->render($node->getActivity(),'view.tpl') ?>
	</fieldset>

	<? if ($node->isPermitted('write')) { ?>
	<form action="<?= tpl_link('issue','edit',$node->nodeId) ?>" method="get">
		<input type="hidden" name="stack[]" value="<?= tpl_uri_call() ?>" />
		<p><input accesskey="e" title="<?= $this->text('Keyboard shortcut: Alt+%s','E') ?>" type="submit" value="<?= $this->text('Edit') ?>" /></p>
	</form>
	<? } ?>
</div>
