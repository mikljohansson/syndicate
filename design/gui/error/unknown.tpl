<? 
if (!headers_sent())
	header("HTTP/1.0 $status error: Unknown Error");
$this->assign('title', "$status Unknown Error");
?>
<h1><?= $status ?> Unknown Error</h1>
<? if (null != $message) { ?>
	<b><?= tpl_text('Note:') ?></b>
	<div style="margin-left:1em;"><?= $message ?></div>
<? } ?>
<? $this->display(tpl_design_path('gui/error/exception.tpl')) ?>
