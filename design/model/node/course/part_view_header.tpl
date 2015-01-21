<div class="Header">
	<h1><a href="<?= tpl_link($node->getHandler(),'view',$node->getCourseId()) ?>"><?= $node->getTitle() ?></a></h1>
	<? if ($node->isPermitted('write')) { ?>
	<div class="Info">
		<? $this->render(SyndNodeLib::getInstance($node->data['UPDATE_NODE_ID']),'head_view.tpl') ?>;
		<?= ucwords(tpl_strftime('%A, %d %B %Y %H:%M', $node->data['TS_UPDATE'])) ?>;
		<a href="<?= $this->call($node->getHandler(),'edit',$node->nodeId) ?>"><?= tpl_text('Edit this course') ?></a>
	</div>
	<? } ?>
</div>