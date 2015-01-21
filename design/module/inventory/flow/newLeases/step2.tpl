<form method="post">
	<h2><?= tpl_text('Batch create new leases') ?></h2>
	<div class="indent">
		Fill in a list of client logins or social security numbers. Use any non <br />
		alphanumerical characters as separator. For example:
		<br /><br />
		<div class="indent">
			<code>
				di99ohmi, ohrn, it2stud<br />
				8123456789<br />
				8212345678
			</code>
		</div>
	</div>
	<br />

	<h3><?= tpl_text('Prototype lease') ?></h3>
	<div class="indent">
		<? $this->render($prototype,'list_view_duration.tpl') ?>
	</div>
	<br />


	<textarea name="query" cols="70" rows="12"><?= $request['query'] ?></textarea>
	<br /><br />
	<input type="submit" name="post" value="<?= tpl_text('Proceed') ?> >>>" />
</form>