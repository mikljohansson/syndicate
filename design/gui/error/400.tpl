<? 
if (!headers_sent())
	header('HTTP/1.0 400 error: Bad Request');
$this->assign('title', '400 Bad Request'); 
?>
<h1>400 Bad Request</h1>
<p><?= tpl_text('Your browser sent a request that this server could not understand.') ?></p>
<? if (null != $message) { ?>
	<b><?= tpl_text('Note:') ?></b>
	<div style="margin-left:1em;"><?= $message ?></div>
<? } ?>
<? $this->display(tpl_design_path('gui/error/exception.tpl')) ?>