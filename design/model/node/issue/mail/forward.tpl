<?
$subject = method_exists($content, 'getSubject') ? $content->getSubject() : $node->getTitle();
$subject = preg_replace('/^(\s*\[?(Fwd?|Re):\s*)+|\s*\(fwd\)|[\s\]]+$/i', '', $subject);
$subject = preg_replace('/\s*#\d+\s*$/', '', $subject);

if (false === strpos($subject, $node->getEmailSubjectId()))
	$subject = $this->translate('Fwd: %s %s', $subject, $node->getEmailSubjectId());
else
	$subject = $this->translate('Fwd: %s', $subject);

if (false != tpl_gui_path(get_class($content),'forward.tpl',false))
	$body = $this->fetchnode($content,'forward.tpl');
else {
	$body = "\r\n\r\n".$this->translate('---------- Forwarded message ----------')."\r\n";
	$body .= $this->translate('Date: %s', ucwords(tpl_strftime('%a, %d %b %Y %H:%M:%S %O', $node->data['TS_CREATE'])))."\r\n";
	$body .= $this->translate('From: %s <%s>', $node->getCreator()->toString(), $node->getCreator()->getEmail())."\r\n";
	$body .= "\r\n".preg_replace('/(^|\n)/', '\1> ', $content->toString());
}

?>
<form action="<?= tpl_link_jump($node->getHandler(),'invoke',$node->nodeId,'send') ?>" method="post">
	<h1><?= $this->translate('Forward message') ?></h1>
	<? $this->render($node,'mail/message.tpl',array('to' => $to, 'subject' => $subject, 'body' => $body, 'files' => $files, 
		'redirect' => ($content instanceof synd_type_email)?$content->id():false)) ?>
</form>

<script type="text/javascript">
<!--
	if (document.getElementById)
		document.getElementById('to').focus();
//-->
</script>
