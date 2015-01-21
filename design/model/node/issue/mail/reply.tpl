<?
if (empty($to) && method_exists($content, 'getSender'))
	$to = $content->getSender();
if (empty($to))
	$to = $node->getCreator()->getEmail();
if (empty($to) && method_exists($content, 'getReceiver'))
	$to = $content->getReceiver();

$subject = method_exists($content, 'getSubject') && null != $content->getSubject() ? $content->getSubject() : $node->getTitle();
$subject = preg_replace('/^(\s*\[?(Fwd?|Re):\s*)+|\s*\(fwd\)|[\s\]]+$/i', '', $subject);
$subject = preg_replace('/\s*#\d+\s*$/', '', $subject);

if (false === strpos($subject, $node->getEmailSubjectId()))
	$subject = $this->translate('Re: %s %s', $subject, $node->getEmailSubjectId());
else
	$subject = $this->translate('Re: %s', $subject);

if (false != tpl_gui_path(get_class($content),'reply.tpl',false))
        $body = $this->fetchnode($content,'reply.tpl');
else {
	$body  = "\r\n\r\n\r\n".$this->translate('%s wrote:', $to)."\r\n\r\n";
	$body .= preg_replace('/(^|\n)/', '\1> ', $content->toString());
}

?>
<form action="<?= tpl_link_jump($node->getHandler(),'invoke',$node->nodeId,'send') ?>" method="post">
	<h1><?= $this->translate('Reply to message') ?></h1>
	<? $this->render($node,'mail/message.tpl',array('to' => $to, 'subject' => $subject, 'body' => $body, 'files' => $files)) ?>
</form>

<script type="text/javascript">
<!--
	if (document.getElementById) {
		var oTextarea = document.getElementById('message');
		oTextarea.focus();
		if (oTextarea.setSelectionRange)
			oTextarea.setSelectionRange(0, 0);
		setInterval(function() {document.getElementById('duration').value++;}, 60000);
	}
//-->
</script>
