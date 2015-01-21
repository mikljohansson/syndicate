<div class="Issue">
	<div class="Header">
		<h3><?= $node->getTitle() ?></h3>
		<div class="Info">
			<? $issue = $node->getParentIssue(); if (!$issue->isNull() && $issue->isPermitted('read')) { ?>
			<?= tpl_translate('Issue number #%s, subissue of #<a href="%s">%s</a>', 
				tpl_quote($node->objectId()), 
				tpl_link($issue->getHandler(),'view',$issue->nodeId), 
				tpl_quote(tpl_chop($issue->toString(),25))) ?>
			<? } else { ?>
			<?= tpl_text('Issue number #%s', $node->objectId()) ?>
			<? } ?>
		</div>
	</div>

	<? if ($node->isPermitted('write')) { ?>
	<ul class="Actions">
		<li><a href="<?= tpl_link_jump($node->getHandler(),'edit',$node->nodeId) ?>">
			<?= tpl_text('Edit issue') ?></a></li>
		<? if (!$node->isClosed()) { ?>
		<li><a href="<?= tpl_link_jump($node->getHandler(),'invoke',$node->nodeId,'cancel') ?>">
			<?= tpl_text('Cancel issue') ?></a></li>
		<? } ?>
	</ul>
	<? } ?>

	<div>
		<b><?= tpl_text('Client') ?></b> 
		<? $this->render($node->getCustomer(),'contact.tpl') ?>
	</div>

	<? $assigned = $node->getAssigned(); if (!$assigned->isNull()) { ?>
	<div>
		<b><?= tpl_text('Assigned') ?></b> 
		<? $this->render($assigned,'contact.tpl') ?>
	</div>
	<? } ?>

	<? if (!$node->isClosed()) { ?>
	<div>
		<b><?= tpl_text('Due') ?></b> 
		<?= date('Y-m-d', $node->data['TS_RESOLVE_BY']) ?>
	</div>
	<? } ?>

	<? $content = $node->getContent(); if (null != trim($content->toString())) { ?>
	<h4><?= tpl_text('Description') ?></h4>
	<div class="Body">
		<? $this->render($content,'full_view.tpl') ?>
	</div>
	<? } ?>

	<? if (count($issues = SyndLib::filter($node->getChildren(),'isPermitted','read'))) { ?>
	<h4><?= tpl_text('Subissues') ?></h4>
		<? foreach (array_keys($issues) as $key) { ?>
		<? $this->render($issues[$key],'head_view.tpl') ?><br />
		<? } ?>
	<? } ?>

	<? if (count($node->getFiles())) { ?>
	<h4><?= tpl_text('Files') ?></h4>
	<? $this->iterate($node->getFiles(),'list_view.tpl') ?>
	<? } ?>

	<? if (count($node->getNotes())) { ?>
	<h4><?= tpl_text('Comments') ?></h4>
	<? $this->iterate($node->getNotes(),'full_view.tpl') ?>
	<? } ?>

	<? if ($node->getDuration()) { ?>
	<div><b><?= tpl_text('Time spent') ?></b> <?= tpl_duration($node->getDuration()) ?></div>
	<? } ?>
	<? if ($node->getEstimate()) { ?>
	<div><b><?= tpl_text('Time estimate') ?></b> <?= tpl_duration($node->getEstimate()) ?></div>
	<? } ?>
</div>
