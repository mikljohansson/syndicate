<? global $synd_user; ?>
<div class="Article">
	<? $this->render($node,'part_view_header.tpl') ?><br />
	<? $this->display('model/node/page/table.tpl',array('list'=>$node->getProgressAttempts($synd_user))) ?>
</div>