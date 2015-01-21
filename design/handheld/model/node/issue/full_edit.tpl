<? global $synd_user; $mailNotifier = $node->getMailNotifier(); ?>
<? include tpl_design_path('gui/errors.tpl'); ?>

<h4><?= tpl_text('Project') ?><? if (isset($errors['PARENT_NODE_ID'])) print '<span style="color:red;">*</span>' ?></h4>
<? $options = $node->getParentOptions(); ?>
<select name="data[PARENT_NODE_ID]" style="width:200px;">
	<? $this->iterate(SyndLib::sort($options),
		'option_expand_children.tpl',array('selected' => $node->getParent())) ?>
</select>

<? if ($node->isPermitted('admin') || !$node->isNew()) { ?>
<h4><?= tpl_text('Assigned') ?><? if (isset($errors['ASSIGNED_NODE_ID'])) print '<span style="color:red;">*</span>' ?></h4>
	<? if ($node->isPermitted('admin')) { ?>
		<select tabindex="3" name="data[ASSIGNED_NODE_ID]" style="width:200px;">
			<option value="user_null.null"><?= tpl_text('Unassigned') ?></option>
			<?= tpl_form_options(SyndLib::invoke(SyndLib::sort($node->getAssignedOptions()),'toString'),$assigned->nodeId) ?>
		</select>
	<? } else { ?>
		<a href="<?= tpl_link('user','summary',$assigned->nodeId) ?>"><?= $assigned->toString() ?></a>
	<? } ?>
<? } ?>

<h4><?= tpl_text('Due') ?><? if(isset($errors['TS_RESOLVE_BY'])) print '<span style="color:red;">*</span>'; ?></h4>
<? $date = tpl_date('Y-m-d', $data['TS_RESOLVE_BY'], strtotime('+1 week')); ?>
<input tabindex="2" type="text" name="data[TS_RESOLVE_BY]" value="<?= $date ?>" size="12" />

<input type="hidden" name="data[INFO_STATUS]" value="<?= synd_node_issue::PENDING ?>" />
&nbsp;<?= tpl_form_checkbox('data[INFO_STATUS]',$node->isClosed(),synd_node_issue::CLOSED) ?> 
<label for="data[INFO_STATUS]"><b><?= tpl_text('Closed') ?></b></label><br />

<? if ($node->isPermitted('admin')) { ?>
	<? $customer = $node->getCustomer(); ?>
	<h4><?= tpl_text('Client') ?><? if(isset($errors['client'])) print '<span style="color:red;">*</span>'; ?></h4>
	<input type="hidden" name="data[prevClient]" value="<?= tpl_attribute($data['client']) ?>" />
	<input type="text" name="data[client]" value="<?= tpl_value($data['client'], $customer->getLogin()) ?>" size="23" />
	<? if (isset($errors['client']['matches'])) { ?>
		<div class="Info">
			<input type="radio" name="data[CLIENT_NODE_ID]" value="user_case.<?= tpl_attribute($data['client']) ?>" id="client[case]" />
			<label for="client[case]"><?= tpl_text("Text <em>'%s'</em> only", $data['client']) ?></label><br />
			<? foreach (array_keys($clients = $errors['client']['matches']) as $key) { ?>
			<input type="radio" name="data[CLIENT_NODE_ID]" value="<?= $clients[$key]->nodeId ?>" id="client[<?= $clients[$key]->nodeId ?>]" />
			<label for="client[<?= $clients[$key]->nodeId ?>]"><? $this->render($clients[$key],'contact.tpl') ?></label><br />
			<? } ?>
		</div>
	<? } ?>
<? } ?>

<h4><?= tpl_text('Title') ?><? if(isset($errors['INFO_HEAD'])) print '<span style="color:red;">*</span>'; ?></h4>
<input type="text" name="data[INFO_HEAD]" value="<?= tpl_value($data['INFO_HEAD']) ?>" size="23" />

<h4><?= tpl_text('Description') ?><? if(isset($errors['content'])) print '<span style="color:red;">*</span>'; ?></h4>
<?= tpl_form_textarea('data[content]',$data['content'],array('cols'=>24),8) ?>

<h4><?= tpl_text('Notify') ?></h4>
<?= tpl_form_checkbox('data[mail][client]', $data['mail']['client']) ?> 
	<label for="data[mail][client]"><?= tpl_text('Client') ?></label>&nbsp;&nbsp;
<?= tpl_form_checkbox('data[mail][assigned]', $data['mail']['client']) ?> 
	<label for="data[mail][assigned]"><?= tpl_text('Assigned') ?></label>&nbsp;&nbsp;

<h4><?= tpl_text('Attach note') ?></h4>
<textarea tabindex="7" name="data[task][content]" cols="24" rows="3"><?= $data['task']['content'] ?></textarea><br />

<div style="margin:0.5em 0 0.5em 0;">
	<input accesskey="s" type="submit" name="post" value="<?= $node->isNew()?tpl_text('Add issue'):tpl_text('Save') ?>" />
	<input type="button" value="<?= tpl_text('Abort') ?>" onclick="window.location='<?= tpl_uri_return() ?>'" />
	&nbsp;&nbsp;<input type="text" name="data[task][INFO_DURATION]" 
		value="<?= tpl_def($request['data']['task']['INFO_DURATION'], 20) ?>" size="3" /> 
	&nbsp;<b><?= tpl_text('Time spent') ?></b>
</div>

<? $this->iterate($node->getNotes(),'full_view.tpl') ?>
