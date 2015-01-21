<div>
	<?= tpl_date('Y-m-d H:i', $node->getCreated()) ?> - <?= round($node->getSize()/1000000,2) ?>Mb <br />
	<a href="<?= $node->uri() ?>"><?= tpl_chop($node->toString(),20) ?></a>
</div>