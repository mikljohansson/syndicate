<? include $this->path('synd_node_attempt','check.js'); ?>

<div class="Article">
	<? $this->render($node,'part_view_header.tpl') ?><br />
	<? 
	$questionNumber = 1;
	if (isset($request['attempts'])) {
		$this->iterate(
			SyndNodeLib::getInstances($request['attempts']),'full_view.tpl',
			array('questionNumber' => &$questionNumber));
	}
	else {
		$attempts = $node->createDiagnosticTest(); 
		$ids = SyndLib::collect($attempts,'nodeId'); ?>
		<form action="<?= tpl_link('system','mplex') ?>" method="post">
			<input type="hidden" name="stack[0]" value="<?= tpl_view($node->getHandler(),'view',$node->nodeId,'diagnostic',array('attempts'=>$ids)) ?>" />
			<? $this->iterate($attempts,'full_view.tpl', array('questionNumber' => &$questionNumber)) ?>
			
			<br /><br />
			<input type="button" class="button" value="<?= tpl_text('Submit my answers') ?>" 
				onclick="attempt_check_progress(this.form);" />
		</form>
	<? } ?>
	<? $this->render($node,'part_view_footer.tpl') ?>
</div>