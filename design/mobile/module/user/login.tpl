<form action="<?= $_SERVER['REQUEST_URI'] ?>" method="post">
	Username<br />
	<input type="text" name="username" size="8" value="<?= $request['username'] ?>" /><br />
	Password<br />
	<input type="password" name="password" size="8" /><br />
	<input type="submit" value="<?= tpl_text('Login') ?>" /><br />
	
	<input type="checkbox" name="persist" value="1" checked="checked" />
		<?= tpl_text('Remember me') ?>
</form>

<em><?= tpl_text('Note:') ?> <?= tpl_text('To login your webbrowser must have cookies turned on.') ?></em>