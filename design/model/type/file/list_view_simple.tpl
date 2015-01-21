<div class="" style="margin-bottom:5px;">
	<?= tpl_date('Y-m-d', $node->getCreated()) ?> - <?= round($node->getSize()/1000000,1) ?>Mb <br />
	<a href="<?= $node->uri() ?>"><?= tpl_chop($node->toString(),20) ?></a>
</div>