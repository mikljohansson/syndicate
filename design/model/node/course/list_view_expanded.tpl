<ul class="Menu">
	<li class="Expanded">
		<a href="<?= tpl_link($node->getHandler(),'view',$node->getCourseId()) 
			?>" title="<?= tpl_chop($node->getDescription(),50) ?>"><?= tpl_chop($node->toString(),20) ?></a>
		<? 
		$page = $node->getPage();
		if (count($chapters = SyndLib::filter($page->getChapters(),'isPermitted','read'))) { ?>
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
</ul>