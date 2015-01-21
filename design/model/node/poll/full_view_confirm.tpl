<?
if (null != $node->data['INFO_REDIRECT'])
	SyndLib::attachHook('html_head', array($node, '_callback_html_head_redirect'));
?>
<div class="Article">
	<div class="Header" style="margin-bottom:10px;">
		<?= $node->getTitle() ?>
	</div>
	<? if (null != $node->data['INFO_REDIRECT']) { ?>
	<div class="Success">
		<?= tpl_translate('You are being redirected within a few seconds, click <a href="%s">here</a> if you are not redirected automatically.', $node->data['INFO_REDIRECT']) ?>
	</div>
	<? } ?>
	<? 
	$confirm = $node->getConfirmation();
	$body = $confirm->getBody();
	print $body->toString();
	?>
</div>
