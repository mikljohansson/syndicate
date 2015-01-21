<? if ($valid) { ?>
	<? if ($key->getTrust()) { ?>
	<div class="Success">
		<?= tpl_translate('Valid signature from %s <span class="Info">(key id: <a href="%s">%s</a>)</span>', 
			tpl_email($key->getIdentity()->toString()), 
			tpl_link('issue','project',$project->getProjectId(),'admin','crypto',$key->getKeyid()), 
			tpl_quote($key->getKeyid())) ?>
	</div>
	<? } else { ?>
	<div class="Notice">
		<?= tpl_translate('Valid signature from untrusted key %s <span class="Info">(you may specify a trust level for this key at <a href="%s">%s</a>, key id: <a href="%s">%s</a>)</span>', 
			tpl_email($key->getIdentity()->toString()),
			tpl_link('issue','project',$project->getProjectId(),'admin','crypto',$key->getKeyid()), 
			tpl_quote($project->toString()), 
			tpl_link('issue','project',$project->getProjectId(),'admin','crypto',$key->getKeyid()), 
			tpl_quote($key->getKeyid())) ?>
	</div>
	<? } ?>
<? } else if ($key) { ?>
<div class="Warning">
	<? if ($quoted) { ?>
	<?= tpl_translate('Signature verification failed, indented message part was probably modified <span class="Info">(attemped key %s, key id: <a href="%s">%s</a>)</span>', 
		tpl_email($key->getIdentity()->toString()), tpl_link('issue','project',$project->getProjectId(),'admin','crypto',$key->getKeyid()), tpl_quote($key->getKeyid())) ?>
	<? } else { ?>
	<?= tpl_translate('Signature verification failed <span class="Info">(attemped key %s, key id: <a href="%s">%s</a>)</span>', 
		tpl_email($key->getIdentity()->toString()), tpl_link('issue','project',$project->getProjectId(),'admin','crypto',$key->getKeyid()), tpl_quote($key->getKeyid())) ?>
	<? } ?>
</div>
<? } else { ?>
<div class="Notice">
	<? if (empty($keyid)) { ?>
	<?= tpl_translate('Unverified message signature <span class="Info">(you need to upload the senders public key to <a href="%s">%s</a> in order to verify the signature)</span>', 
		tpl_link('issue','project',$project->getProjectId(),'admin','crypto'), 
		tpl_quote($project->toString())) ?>
	<? } else { ?>
	<?= tpl_translate('Unverified message signature <span class="Info">(you need to upload the senders public key to <a href="%s">%s</a> in order to verify the signature, key id: %s)</span>', 
		tpl_link('issue','project',$project->getProjectId(),'admin','crypto'), 
		tpl_quote($project->toString()), 
		tpl_quote($keyid)) ?>
	<? } ?>
</div>
<? } ?>
