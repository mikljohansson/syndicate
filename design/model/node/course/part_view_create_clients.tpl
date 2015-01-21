<div class="indent">
	<?= tpl_text('Creating new clients for %s', $group->toString()) ?>
</div>

<form method="post">
	<h3><?= tpl_text('Note:') ?></h3>
	<div class="indent">
		Fill in a list of email addresses and names. A new account and password 
		will be created for each client and instructions for logging in will be 
		mailed to each address. Clients login using their email address and the 
		password that is mailed to them.
		<br /><br />

		Specify one client per line. The first column should be the email address
		and the second the name. For example: <br /><br />
		<div class="indent">
			<code>
				mikael@example.com Johansson, Mikael<br />
				bradley@example.com Bradley, Linda
			</code>
		</div>
	</div>

	<textarea name="emails" cols="80" rows="10"><?= tpl_value($request['emails']) ?></textarea>

	<br /><br />
	<input type="submit" value="<?= tpl_text('Create accounts') ?>" />
	<input type="button" value="<?= tpl_text('Abort') ?>" onclick="window.location='<?= tpl_uri_return() ?>';" />
</form>

<?
if (isset($request['emails'])) { 
	list ($created, $failed) = $node->createClientAccounts($this, $group, $request['emails']);

	if (count($created)) { ?>
		<br />
		<h3><?= tpl_text('Created') ?></h3>
		<div class="indent">
			<? foreach (array_keys($created) as $key) { ?>
				<?= $created[$key]->toString() ?><br />
			<? } ?>
		</div><? 
	} 

	if (count($failed)) { ?>
		<br />

		<h3><?= tpl_text('Failed to create accounts for') ?>:</h3>
		<table class="indent">
			<? foreach ($failed as $fail) { ?>
			<tr>
				<td><?= $fail[0] ?></td>
				<td style="padding-left:30px;"><?= $fail[1] ?></td>
			</tr>
			<? } ?>
		</table><? 
	} 
} 
?>