<? if ($node->hasChapters()) { ?>
<div class="Navigation">
	<h3><?= tpl_text('Subchapters') ?></h3>
	<? $this->display('model/node/page/table.tpl',array('list'=>SyndLib::filter($node->getChapters(),'isPermitted','read'))) ?>
</div>
<? } ?>