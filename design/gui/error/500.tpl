<? 
if (!headers_sent())
	header('HTTP/1.0 500 error: Internal Server Error');
$this->assign('title', '500 Internal Server Error'); 
?>
<h1>500 Internal Server Error</h1>
<p><?= tpl_text('The server encountered an unexpected condition which prevented it from fulfilling the request.') ?></p>
<? if (null != $message) { ?>
	<b><?= tpl_text('Note:') ?></b>
	<div style="margin-left:1em;"><?= $message ?></div>
<? } ?>
<? $this->display(tpl_design_path('gui/error/exception.tpl')) ?>
