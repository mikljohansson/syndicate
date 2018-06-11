<? tpl_load_script(tpl_design_uri('js/form.js')); ?>
<div class="RequiredField<? if(isset($errors['INFO_HEAD'])) print ' InvalidField'; ?>">
	<h3><?= tpl_text('Name') ?></h3>
	<input type="text" name="data[INFO_HEAD]" match="\w+" message="<?= tpl_text('Please provide a name') ?>" 
		value="<?= tpl_value($data['INFO_HEAD']) ?>" size="64" maxlength="255" />
</div>

<div class="RequiredField<? if(isset($errors['INFO_DESC'])) print ' InvalidField'; ?>">
	<h3><?= tpl_text('Description') ?></h3>
	<?= tpl_form_textarea('data[INFO_DESC]',$data['INFO_DESC'],
		array('cols'=>'48','match'=>'\w+','message'=>tpl_text('Please provide a description'))) ?>
</div>

<div class="OptionalField<? if(isset($errors['PARENT_NODE_ID'])) print ' InvalidField'; ?>">
	<h3><?= tpl_text('Parent project') ?></h3>
	<? $options = SyndLib::sort($node->getParentOptions()); ?>
	<select name="data[PARENT_NODE_ID]" style="width:200px;">
		<option value="">&nbsp;</option>
		<? $this->iterate($options,'option_expand_children.tpl',array('selected' => $node->getParent())) ?>
	</select>
</div>

<div class="OptionalField<? if(isset($errors['INFO_PROJECT_ID'])) print ' InvalidField'; ?>">
	<h3><?= tpl_text("Project identifier") ?></h3>
	<input type="text" name="data[INFO_PROJECT_ID]" value="<?= tpl_value($data['INFO_PROJECT_ID']) ?>" size="64" maxlength="64" />
	<div class="Info"><?= tpl_text('Used in web and email addresses') ?></div>
</div>

<div class="OptionalField<? if(isset($errors['INFO_COST_CENTER'])) print ' InvalidField'; ?>">
	<h3><?= tpl_text("Costcenter") ?></h3>
	<input type="text" name="data[INFO_COST_CENTER]" value="<?= tpl_value($data['INFO_COST_CENTER']) ?>"  size="64" maxlength="64" />
	<div class="Info"><?= tpl_text('A cost center for this project, shown in reports') ?></div>
</div>

<div class="OptionalField<? if(isset($errors['INFO_PROJECT_NUMBER'])) print ' InvalidField'; ?>">
	<h3><?= tpl_text("Project number") ?></h3>
	<input type="text" name="data[INFO_PROJECT_NUMBER]" value="<?= tpl_value($data['INFO_PROJECT_NUMBER']) ?>"  size="64" maxlength="64" />
	<div class="Info"><?= tpl_text('An economic reference number for this project, shown in reports') ?></div>
</div>

<div class="OptionalField<? if(isset($errors['INFO_EMAIL'])) print ' InvalidField'; ?>">
	<h3><?= tpl_text('Email address') ?></h3>
	<input type="text" name="data[INFO_EMAIL]" value="<?= tpl_value($data['INFO_EMAIL']) ?>" size="64" maxlength="255" />
	<div class="Info">
		<?= tpl_text('This address must be specifically configured in the mail server to pass emails directly to the issue module') ?><? 
		if (null != $node->getEmail() && $node->getEmail() != $data['INFO_EMAIL']) { ?>. <?= tpl_text('Leave empty for default <b>%s</b>', $node->getEmail()) ?><? } ?>
	</div>
</div>

<div class="OptionalField<? if(isset($errors['INFO_DEFAULT_CLIENT'])) print ' InvalidField'; ?>">
	<h3><?= tpl_text('Default client (username)') ?></h3>
	<input type="text" name="data[INFO_DEFAULT_CLIENT]" value="<?= tpl_value($data['INFO_DEFAULT_CLIENT']) ?>" size="64" maxlength="255" />
	<div class="Info"><?= tpl_text('Emailed issues lacking a known client uses the sender, this provides a default client') ?></div>
</div>

<div class="OptionalField<? if(isset($errors['INFO_DEFAULT_RESOLVE'])) print ' InvalidField'; ?>">
	<h3><?= tpl_text('Issue due in') ?></h3>
	<input type="text" name="data[INFO_DEFAULT_RESOLVE]" value="<?= tpl_value($data['INFO_DEFAULT_RESOLVE']) ?>" size="64" maxlength="255" />
	<div class="Info"><?= tpl_text('Default time before an issue is due, should be an expression such as "+14 days", "+4 hours" or "Friday +1 week". For more info see <a href="http://www.php.net/strtotime">http://www.php.net/strtotime</a>') ?></div>
</div>

<div class="OptionalField<? if(isset($errors['INFO_DEFAULT_REOPEN'])) print ' InvalidField'; ?>">
	<h3><?= tpl_text('Issue reopen on email') ?></h3>
	<input type="text" name="data[INFO_DEFAULT_REOPEN]" value="<?= tpl_value($data['INFO_DEFAULT_REOPEN']) ?>" size="64" maxlength="255" />
	<div class="Info"><?= tpl_text('Issues closed for more than this time period are not reopened when email arrive, instead a new issue is created. Should be an expression such as "+14 days", "+4 hours" or "Friday +1 week". For more info see <a href="http://www.php.net/strtotime">http://www.php.net/strtotime</a>') ?></div>
</div>

<div class="OptionalField<? if(isset($errors['INFO_CLEANUP_CUTOFF'])) print ' InvalidField'; ?>">
	<h3><?= tpl_text('GDPR cleanup timeframe') ?></h3>
	<input type="text" name="data[INFO_CLEANUP_CUTOFF]" value="<?= tpl_value($data['INFO_CLEANUP_CUTOFF']) ?>" size="64" maxlength="255" />
	<div class="Info"><?= tpl_text('Issues older than this cutoff will automatically have their personally identifable information removed. Should be an expression such as "-5 years" or "-18 months". For more info see <a href="http://www.php.net/strtotime">http://www.php.net/strtotime</a>') ?></div>
</div>

<?= SyndLib::runHook('project_edit', $node) ?>

<div class="OptionalField">
	<?= tpl_form_checkbox('data[FLAG_INHERIT_MEMBERS]',$data['FLAG_INHERIT_MEMBERS']) ?>
		<label for="data[FLAG_INHERIT_MEMBERS]"><?= tpl_text('Inherit project members from parent project') ?></label><br />
	<?= tpl_form_checkbox('data[FLAG_INHERIT_CATEGORIES]',$data['FLAG_INHERIT_CATEGORIES']) ?>
		<label for="data[FLAG_INHERIT_CATEGORIES]"><?= tpl_text('Inherit categories from parent project') ?></label><br />
	<?= tpl_form_checkbox('data[FLAG_HIDE_ISSUES]',$data['FLAG_HIDE_ISSUES']) ?>
		<label for="data[FLAG_HIDE_ISSUES]"><?= tpl_text('Hide issues from listings until they are overdue') ?></label><br />
	<?= tpl_form_checkbox('data[FLAG_DISCARD_SPAM]',$data['FLAG_DISCARD_SPAM']) ?>
		<label for="data[FLAG_DISCARD_SPAM]"><?= tpl_text('Discard spam flagged email (the X-Spam-Status header)') ?></label><br />
	<?= tpl_form_checkbox('data[FLAG_DISPLAY_SENDER]',$data['FLAG_DISPLAY_SENDER']) ?>
		<label for="data[FLAG_DISPLAY_SENDER]"><?= tpl_text('Logged in user as sender on outgoing emails') ?></label><br />
	<?= tpl_form_checkbox('data[FLAG_ISSUE_SENDER]',$data['FLAG_ISSUE_SENDER']) ?>
		<label for="data[FLAG_ISSUE_SENDER]"><?= tpl_text('Encode issue number on email address in outgoing emails') ?></label><br />
	<?= tpl_form_checkbox('data[FLAG_RECEIPT]',$data['FLAG_RECEIPT']) ?>
		<label for="data[FLAG_RECEIPT]"><?= tpl_text('Send confirmation e-mail when new issues are created') ?></label><br />
	<?= tpl_form_checkbox('data[FLAG_ARCHIVE]',$data['FLAG_ARCHIVE']) ?>
		<label for="data[FLAG_ARCHIVE]"><?= tpl_text('Project is archived and hidden from view') ?></label><br />
	<?= SyndLib::runHook('project_edit_options', $node) ?>
</div>
