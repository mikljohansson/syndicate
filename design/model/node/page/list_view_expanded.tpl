<? global $synd_user; $parent = $node->getParent(); ?>
<li class="<?= $node->hasChapters() ? 'Expanded' : 'Leaf' ?>">
	<? if (null != ($attempt = $node->getBestProgressAttempt($synd_user))) { ?>
	<img class="ProgressIndicator" src="<?= tpl_design_uri('image/pixel.gif') ?>" style="background-color:<?= $attempt->getColor() ?>;" />
	<? } ?>
	<a href="<?= tpl_link('course','view',$node->nodeId) ?>" title="<?= tpl_attribute($node->getPageNumber().' '.$node->toString()) ?>"><? 
		$course = Module::getInstance('course'); $topNode = $course->getActiveNode(); 
		if ($topNode->nodeId == $node->nodeId) { 
			?><u><?= tpl_chop($node->toString(),20) ?></u><? 
		} else { 
			?><?= tpl_chop($node->toString(),20) ?><? 
		} 
	?></a>
	<? if (count($chapters = SyndLib::filter($node->getChapters(),'isPermitted','read'))) { ?>
		<ul>
		<? foreach (array_keys($chapters) as $key) { ?>
			<? if (isset($branch[$chapters[$key]->nodeId])) { ?>
				<? $this->render($chapters[$key],'list_view_expanded.tpl',$_data) ?>
			<? } else { ?>
				<? $this->render($chapters[$key],'list_view_collapsed.tpl') ?>
			<? } ?>
		<? } ?>
		</ul>
	<? } ?>
</li>
