	<? if (count($files = $node->getFiles())) { ?>
	<h3><?= tpl_text('Files') ?></h3>
		<? if (count($thumbs = array_slice(SyndLib::filter(SyndLib::sort($files,'getCreated'),'hasImage'),0,9))) { ?>
			<? $files = SyndLib::array_kdiff($files, $thumbs); ?>
			<? $this->iterate($thumbs,'thumb.tpl') ?>
		<? } ?>
		<?= tpl_gui_table('file',$files,'view_simple.tpl') ?>
	<? } ?>