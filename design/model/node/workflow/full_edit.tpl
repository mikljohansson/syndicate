<? $this->assign('workflow', $node) ?>
<div class="FlowingForm">
	<fieldset>
		<legend><?= $this->text('Workflow') ?></legend>
		<div class="Information">
			<h4><?= $this->text('Workflow') ?></h4>
			<p><?= $this->text('The accesskey may be used as a keyboard shortcut when the workflow is displayed in the sidebar') ?></p>
		</div>
		<div class="Required<?= isset($errors['PARENT_NODE_ID'])?' Invalid':'' ?>">
			<label for="data[PARENT_NODE_ID]"><?= $this->text('Project') ?></label>
			<select name="data[PARENT_NODE_ID]">
				<? $this->iterate(SyndLib::sort($node->getParentOptions()),'option_expand_children.tpl',
					array('selected'=>$node->getParent())) ?>
			</select>
		</div>
		<div class="Required<?= isset($errors['INFO_HEAD'])?' Invalid':'' ?>">
			<label for="data[INFO_HEAD]"><?= $this->text('Name') ?></label>
			<?= tpl_form_text('data[INFO_HEAD]', $data['INFO_HEAD']) ?>
		</div>
		<div class="Optional<?= isset($errors['INFO_DESC'])?' Invalid':'' ?>">
			<label for="data[INFO_DESC]"><?= $this->text('Description') ?></label>
			<textarea name="data[INFO_DESC]" id="data[INFO_DESC]"><?= $this->quote($data['INFO_DESC']) ?></textarea>
		</div>
		<div class="Optional<?= isset($errors['FLAG_CONTEXT_MENU'])||isset($errors['FLAG_SIDEBAR_MENU'])?' Invalid':'' ?>">
			<label><?= $this->text('Display') ?></label>
			<p>
				<?= tpl_form_checkbox('data[FLAG_CONTEXT_MENU]',$data['FLAG_CONTEXT_MENU']) ?>
				<label for="data[FLAG_CONTEXT_MENU]"><?= $this->text('Display in context menu') ?></label><br />
				<?= tpl_form_checkbox('data[FLAG_SIDEBAR_MENU]',$data['FLAG_SIDEBAR_MENU']) ?>
				<label for="data[FLAG_SIDEBAR_MENU]"><?= $this->text('Display in sidebar') ?></label>
			</p>
		</div>
		<div class="Optional<?= isset($errors['INFO_ACCESSKEY'])?' Invalid':'' ?>">
			<label for="data[INFO_ACCESSKEY]"><?= $this->text('Accesskey') ?></label>
			<input type="text" name="data[INFO_ACCESSKEY]" id="data[INFO_ACCESSKEY]" value="<?= $this->quote($data['INFO_ACCESSKEY']) ?>" />
			<small><?= $this->text('The accesskey is used as Alt+Shift+&lt;key&gt; or Alt+&lt;key&gt; depending on the web browser.') ?></small>
		</div>
	</fieldset>

	<fieldset>
		<legend><?= $this->text('Events') ?></legend>
		<div class="Information">
			<h4><?= $this->text('Events') ?></h4>
			<p><?= $this->text('Workflows may optionally be triggered automatically when events occur, please select any events that should trigger the workflow.') ?></p>
			<p><?= $this->text('The workflow is triggered for all users when events occur in the specified project and all child projects that inherit the workflow.') ?></p>
		</div>
		<div class="Optional<?= isset($errors['events'])?' Invalid':'' ?>">
			<table style="width:70%;">
				<? foreach ($node->getEventOptions() as $event) { ?>
				<tr>
					<td>
						<?= tpl_form_checkbox('data[events][]',
							in_array(get_class($event),(array)$data['events']),
							get_class($event),'data[events]['.get_class($event).']') ?>
					</td>
					<td><label for="data[events][<?= get_class($event) ?>]"><?= $this->text($event->__toString()) ?></label></td>
					<td><?= $this->text($event->getDescription()) ?></td>
				</tr>
				<? } ?>
			</table>
		</div>
	</fieldset>
	
	<fieldset>
		<legend><?= $this->text('Actions') ?></legend>
		<? $this->render($node->getActivity(),'edit.tpl') ?>
	</fieldset>

	<p>
		<input accesskey="s" title="<?= $this->text('Keyboard shortcut: Alt+%s','S') ?>" type="submit" name="post" value="<?= $this->text('Save') ?>" />
		<input accesskey="a" title="<?= $this->text('Keyboard shortcut: Alt+%s','A') ?>" type="button" value="<?= $this->text('Abort') ?>" onclick="window.location='<?= tpl_uri_return() ?>';" />
		<input type="button" value="<?= $this->text('Delete') ?>" onclick="window.location='<?= tpl_link_jump('issue','delete',$node->nodeId) ?>';" />
	</p>
</div>
