<? global $synd_user; ?>
<div class="Article">
	<? $this->render($node,'part_view_header.tpl') ?>
</div>
<br />

<h3><?= tpl_text('Testresults for %s', $node->toString()) ?></h3>
<div class="list indent">
	<? if (count($attempts = $node->getClientAttempts($synd_user))) { ?>
		<? $this->display('model/node/page/table.tpl',array('list'=>$attempts)) ?>
	<? } else { ?>
		<em><?= tpl_text('No testresults found') ?></em>
	<? } ?>
</div>

