<div class="Article">
	<div class="Header">
		<a id="<?= $node->getPageNumber() ?>"></a>
		<?= $node->getPageNumber() ?> <?= $node->data['INFO_HEAD'] ?>
	</div>
	<div class="Body">
		<?= $node->data['INFO_BODY']->toString() ?>
	</div>
	<? $this->iterate(SyndLib::filter($node->getChapters(),'isPermitted','read'),'part_view_print.tpl') ?>
</div>