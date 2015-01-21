<?= tpl_text('Do you really want to remove all the %d replies to this poll?', $node->getAttemptCount()) ?>
<div class="indent">
	<?= $node->toString() ?>
</div>
<br />

<form method="post">
	<input type="submit" name="confirm" value="<?= tpl_text('Ok') ?>" />
	<input type="button" value="<?= tpl_text('Abort') ?>" onclick="window.location='<?= tpl_uri_return() ?>';" />
</form>