<? include $this->path('synd_node_attempt','check.js'); ?>

<div class="Article">
	<? $this->render($node,'part_view_header.tpl') ?><br />
	<? 
	if (isset($request['attempt'])) 
		print $this->fetchnode(SyndNodeLib::getInstance($request['attempt']),'full_view.tpl',$_data);
	else {
		$attempt = $node->createProgressAttempt(); ?>
		<form action="<?= tpl_link('system','mplex') ?>" method="post">
			<input type="hidden" name="stack[0]" value="<?= tpl_view($node->getHandler(),'view',$node->nodeId,'progress',array('attempt'=>$attempt->nodeId)) ?>" />
			<? $this->render($attempt,'full_view.tpl',$_data) ?>

			<br /><br />
			<input type="button" class="button" value="<?= tpl_text('Submit my answers') ?>" 
				onclick="attempt_check_progress(this.form);" />
		</form>
	<? } ?>
	<? $this->render($node,'part_view_footer.tpl') ?>
</div>