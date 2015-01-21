<? if (!$key->getTrust()) { ?>
<div class="Notice">
	<?= tpl_translate('This key is not trusted and it might not actually belong to %s, before flagging the key as trusted please contact its owner to verify authenticity using the key fingerprint. If you understand the security implications you might flag the key as <u><a href="%s">trusted</a></u> now.', 
		tpl_quote($key->getIdentity()->getName()(, 
		tpl_link_call('issue','crypto','setTrust',$node->nodeId,$key->getKeyid(),(int)!$key->getTrust())) ?>
</div>
<? } ?>

<h3><?= tpl_text('Key properties') ?></h3>
<table class="Vertical">
	<tr>
		<th><?= tpl_text('Fingerprint') ?></th>
		<td><?= chunk_split($key->getFingerprint(),4) ?></td>
	</tr>
	<tr>
		<th><?= tpl_text('Key ID') ?></th>
		<td><?= $key->getKeyid() ?></td>
	</tr>
	<tr>
		<th><?= tpl_text('Type') ?></th>
		<td><?
			if ($key->isEncryptionKey() && $key->isDecryptionKey())
				print tpl_text('Public/Decret');
			else if ($key->isEncryptionKey())
				print tpl_text('Public');
			else if ($key->isDecryptionKey())
				print tpl_text('Secret');
			?></td>
	</tr>
	<tr>
		<th><?= tpl_text('Created') ?></th>
		<td><?= tpl_strftime('%Y-%d-%m', $key->getCreated()) ?></td>
	</tr>
	<tr>
		<th><?= tpl_text('Expiry') ?></th>
		<td><?= tpl_strftime('%Y-%d-%m', $key->getExpiry()) ?></td>
	</tr>
	<tr>
		<th><?= tpl_text('Revoked') ?></th>
		<td><?= $key->isRevoked() ? tpl_text('Yes') : tpl_text('No') ?></td>
	</tr>
	<tr>
		<th><?= tpl_text('Trusted') ?></th>
		<td>
			<? if ($key->getTrust()) { ?>
			<?= tpl_text('Yes') ?> <span class="Info">(<a href="<?= tpl_link_call('issue','crypto','setTrust',$node->nodeId,$key->getKeyid(),(int)!$key->getTrust()) ?>"><?= tpl_text('Not trusted?') ?></a>)</span>
			<? } else print tpl_text('No'); ?>
	</tr>
</table>

<ul class="Actions">
	<li><a href="<?= tpl_link_call('issue','crypto','delete',$node->nodeId,$key->getKeyid()) 
		?>"><?= tpl_text('Delete this key permanently') ?></a></li>
</ul>

<h3><?= tpl_text('Identities') ?></h3>
<? foreach ($key->getIdentities() as $identity) { ?>
	<?= tpl_email($identity->toString()) ?><br />
<? } ?>
