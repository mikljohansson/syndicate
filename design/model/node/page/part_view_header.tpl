<? $node->_metaAttach() ?>
<div class="Breadcrumbs">
	<? 
	$list = $node->getBranch();
	array_pop($list);
	foreach (array_keys($list) as $key) { ?>
		<? $this->render($list[$key],'head_view.tpl') ?> 
		<? if ($i++ < count($list)-1) print '&raquo;'; ?>
	<? } ?>
</div>

<div class="Header">
	<h1><span class="PageNumber"><?= trim($node->getPageNumber(),'.') ?> </span><?= $node->data['INFO_HEAD'] ?></h1>
	<? if ($node->isPermitted('write')) { ?>
	<div class="Info">
		<? $this->render(SyndNodeLib::getInstance($node->data['UPDATE_NODE_ID']),'head_view.tpl') ?>;
		<?= ucwords(tpl_strftime('%A, %d %B %Y %H:%M', $node->data['TS_UPDATE'])) ?>;
		<a href="<?= $this->call($node->getHandler(),'edit',$node->nodeId) ?>"><?= tpl_text('Edit this page') ?></a>
	</div>
	<? } ?>
</div>