<div class="Footer">
	<table>
		<tr>
			<td class="left" width="33%">
				<? if (null != ($prev = $node->getPrevious())) { ?>
				<a href="<?= tpl_link($prev->getHandler(),'view',$prev->nodeId) ?>" title="<?= tpl_text("View the previous page") ?>">&lt;- <?= tpl_text('Previous') ?></a><br />
				<?= $prev->toString() ?>
				<? } else print '&nbsp;'; ?>
			</td>
			<td class="center" width="34%">
				<? $parent = $node->getParent(); ?>
				<? if (!$parent->isNull()) { ?>
				<a href="<?= tpl_link($parent->getHandler(),'view',$parent->nodeId) ?>" title="<?= tpl_text("View this page's parent section") ?>"><?= tpl_text('Up') ?></a>
				<? } else print '&nbsp;'; ?>
			</td>
			<td class="right" width="33%">
				<? if ((($next = $node->getNext()) instanceof synd_node_page)) { ?>
				<a href="<?= tpl_link($next->getHandler(),'view',$next->nodeId) ?>" title="<?= tpl_text("View the next page") ?>"><?= tpl_text('Next') ?> -&gt;</a><br />
				<?= $next->toString() ?>
				<? } else print '&nbsp;'; ?>
			</td>
		</tr>
	</table>
</div>