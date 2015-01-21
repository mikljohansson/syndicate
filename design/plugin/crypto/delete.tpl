<form action="<?= tpl_link_jump('issue','crypto','delete',$node->nodeId,$key->getKeyid()) ?>" method="post">
	<h1><?= tpl_text('Delete key') ?></h1>
	<p><?= tpl_translate('Do you want to delete the key %s <span class="Info">(Key ID: %s)</span>', tpl_email($key->getIdentity()->toString()), tpl_quote($key->getKeyid())) ?></p>
	<p>
		<input type="submit" name="confirm" value="<?= tpl_text('Ok') ?>" />
		<input type="button" value="<?= tpl_text('Abort') ?>" onclick="window.location='<?= tpl_uri_return() ?>';" />
	</p>
</form>
