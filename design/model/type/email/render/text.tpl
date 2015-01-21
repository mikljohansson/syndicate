<? if ($headers !== false) { ?>
<? if (null != ($date = $message->getHeader('Date'))) { ?>
<?= tpl_translate('Date: %s', $date) ?>

<? } ?>
<? if (null != ($from = $message->getHeader('From'))) { ?>
<?= tpl_translate('From: %s', $from) ?>

<? } ?>
<? if (null != ($to = $message->getHeader('To'))) { ?>
<?= tpl_translate('To: %s', $to) ?>

<? } ?>
<? if (null != ($subject = $message->getHeader('Subject'))) { ?>
<?= tpl_translate('Subject: %s', $subject) ?>

<? } } ?>

<?

if (null !== $node->_content) 
	$body = $node->_content;
else if (count($parts = $message->getParts()) && 'multipart/alternative' != strtolower($message->getHeader('Content-Type'))) {
	$body = '';
	foreach (array_keys($parts) as $key) {
		$body .= $this->fetchnode($node,'render/text.tpl',array('message'=>$parts[$key]));
	}
}
else if (!$message->isAttachment())
	$body = $message->getContent();

print empty($quote) ? trim($body) : preg_replace('/(^|\n)/', '\1> ', trim($body));
