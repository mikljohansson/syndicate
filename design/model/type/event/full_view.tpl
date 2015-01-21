<div style="margin-bottom:0.2em;">
	<? $user = $node->getUser(); if ($user->isNull()) { ?>
	<?= tpl_text($node->toString()) ?>
	<? } else { ?>
	<?= tpl_translate('%s by %s', tpl_text($node->toString()), $this->fetchnode($node->getUser(),'contact.tpl')) ?>
	<? } ?>
	<span class="Info">(<?= ucwords(tpl_strftime('%A, %d %b %Y %H:%M',$node->getTimestamp())) ?>)</span>
</div>
