<div class="Item">
	<div class="Header">
		<a href="<?= tpl_link($node->getHandler(),'view',$node->nodeId) ?>">
		<?= $node->data['INFO_HEAD'] ?></a>
	</div>
	<? if ($node->isPermitted('write')) { ?>
	<div class="Info">
		<? $this->render(SyndNodeLib::getInstance($node->data['UPDATE_NODE_ID']),'head_view.tpl') ?> |
		<?= ucwords(tpl_strftime('%A, %d %B %Y %H:%M', $node->data['TS_UPDATE'])) ?> |
		<a href="<?= tpl_link_call($node->getHandler(),'edit',$node->nodeId) ?>">
		<?= tpl_text('Edit this page') ?></a>
	</div>
	<? } ?>
	<? if (null != $node->data['INFO_BODY']) { ?>
	<div class="Abstract">
		<? if (isset($highlight)) { ?>
			<? 
			require_once 'core/lib/SyndHTML.class.inc';
			print tpl_def(SyndHTML::getContextSummary(strip_tags($node->data['INFO_BODY']->toString()), $highlight), 
				tpl_chop(strip_tags($node->data['INFO_BODY']->toString()), 200));
			?>
		<? } else { ?>
			<?= tpl_chop(strip_tags($node->data['INFO_BODY']->toString()), 200) ?>
		<? } ?>
	</div>
	<? } ?>
</div>
