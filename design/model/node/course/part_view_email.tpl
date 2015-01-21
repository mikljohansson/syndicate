<? global $synd_user; ?>
<div class="indent">
	<?= tpl_text('Send email to %s', $group->toString()) ?>
	<div class="Info">
		<?= tpl_text("Reply to address will be '%s'", $synd_user->getEmail()) ?>
	</div>
</div>

<? if (isset($request['result'])) { ?>
	<? if ('sent' == $request['result']) { ?>
		<div class="Success"><?= tpl_text('Email sucessfully sent to %s', $group->toString()) ?></div>
	<? } else if ('recipients' == $request['result']) { ?>
		<div class="Notice"><?= tpl_text('No recipients found, email not sent.') ?></div>
	<? } else { ?>
		<div class="Warning">
			<h3><?= tpl_text('An error occurred') ?></h3>
			<?= tpl_text('Email was not sent, contact your systems administrator if the problem persists.') ?>
		</div>
	<? } ?>
<? } ?>

<form action="<?= tpl_link_call($group->getHandler(),'invoke',$group->nodeId,'email') ?>" method="post">
	<h3><?= tpl_text('Subject') ?></h3>
	<div class="indent">
		<input type="text" name="subject" size="105" maxlength="255" />
	</div>

	<h3><?= tpl_text('Message') ?></h3>
	<div class="indent">
		<textarea name="body" cols="80" rows="10"></textarea><br />
	</div>

	<div class="indent">
		<input type="checkbox" name="recurse" id="recurse" value="1" checked="checked" />
			<label for="recurse"><?= tpl_text('Send mail to members in subgroups of %s too', $group->toString()) ?></label>
	</div>
	
	<input type="submit" value="<?= tpl_text('Send') ?>" onclick="this.disabled=true; this.value='<?= tpl_text('Sending email ...') ?>';" />
</form>
