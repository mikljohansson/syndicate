<h1><?= tpl_text('Retrieve forgotten password') ?></h1>
<? if ($error) { ?>
<p class="Warning"><?= $error ?></p>
<? } ?>
<? if ($status) { ?>
<p class="Result"><?= $status ?></p>
<? } ?>

<form action="<?= tpl_link('user','password') ?>" method="post">
	<h3><?= tpl_text('Enter your username or email address') ?></h3>
	<div class="indent">
		<input type="text" name="username" value="<?= tpl_value($request['username']) ?>" />
		<input type="submit" value="<?= tpl_text('Send') ?>" />
	</div>
</form>