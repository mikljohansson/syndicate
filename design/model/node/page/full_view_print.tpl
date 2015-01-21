<? if (count($chapters = SyndLib::filter($node->getChapters(),'isPermitted','read'))) { ?>
<table style="page-break-after:always;">
	<? $this->iterate($node->getChapters(),'part_view_index.tpl',array('levels'=>1)) ?>
</table>
<? } ?>
<? $this->render($node,'part_view_print.tpl') ?>
