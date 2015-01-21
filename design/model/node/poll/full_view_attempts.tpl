<div style="margin-bottom:1em;">
	<a href="<?= tpl_link($node->getHandler()) ?>"><?= tpl_text('Home') ?></a> &raquo;
	<a href="<?= tpl_link($node->getHandler(),'view',$node->nodeId) ?>"><?= $node->toString() ?></a>
</div>

<div class="indent">
	<?= $node->toString() ?>
</div>
<br />

<? $this->iterate($node->getAttempts(), 'list_view.tpl') ?>