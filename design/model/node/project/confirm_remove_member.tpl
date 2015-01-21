<form method="post">
	<div class="Notice">
		<h1><?= tpl_text('Please confirm') ?></h1>
		<p><?= tpl_text("Do you really want to remove the user <em>'%s'</em> from the project <em>'%s'</em>?",$user->toString(),$node->toString()) ?></p>
		<p>
			<input type="submit" name="confirm" value="<?= tpl_text('Ok') ?>" />
			<input type="button" value="<?= tpl_text('Abort') ?>" onclick="window.location='<?= tpl_uri_return() ?>';" />
		</p>
	</div>
</form>