<? $user = $node->getUser(); ?>
<div style="margin-bottom:0.2em;">
	<? if ($user->isNull()) { ?>
	<?= tpl_translate('Moved to project %s', $this->fetchnode($node->getProject(),'head_view.tpl')) ?>
	<? } else { ?>
	<?= tpl_translate('Moved to project %s by %s', $this->fetchnode($node->getProject(),'head_view.tpl'), $this->fetchnode($user,'contact.tpl')) ?>
	<? } ?>
	<span class="Info">(<?= ucwords(tpl_strftime('%A, %d %b %Y %H:%M',$node->_ts)) ?>)</span>
</div>
