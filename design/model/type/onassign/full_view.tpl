<? $user = $node->getUser(); ?>
<div style="margin-bottom:0.2em;">
	<? if (null != ($assigned = $node->getAssigned()) && !$assigned->isNull()) { ?>
		<? if ($user->isNull()) { ?>
		<?= tpl_translate('Assigned to %s', $this->fetchnode($assigned,'contact.tpl')) ?>
		<? } else { ?>
		<?= tpl_translate('Assigned to %s by %s', $this->fetchnode($assigned,'contact.tpl'), $this->fetchnode($user,'contact.tpl')) ?>
		<? } ?>
	<? } else { ?>
		<?= tpl_translate('Set as unassigned by %s', $this->fetchnode($user,'contact.tpl')) ?>
	<? } ?>
	<span class="Info">(<?= ucwords(tpl_strftime('%A, %d %b %Y %H:%M',$node->_ts)) ?>)</span>
</div>
