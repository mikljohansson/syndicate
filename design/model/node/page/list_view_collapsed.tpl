<? global $synd_user; ?>
<li class="<?= $node->hasChapters() ? 'Collapsed' : 'Leaf' ?>">
	<? if (null != ($attempt = $node->getBestProgressAttempt($synd_user))) { ?>
	<img class="ProgressIndicator" src="<?= tpl_design_uri('image/pixel.gif') ?>" style="background-color:<?= $attempt->getColor() ?>;" />
	<? } ?>
	<a href="<?= tpl_link('course','view',$node->nodeId) ?>" title="<?= tpl_attribute($node->getPageNumber().' '.$node->toString()) ?>"><?= tpl_chop($node->toString(),20) ?></a>
</li>