<div class="page_column_medium">
	<div class="page_text_header">Moderation queue</div>
	<? foreach (array_keys($toModerate) as $key) { ?>
	<div style="margin-bottom:3px;">
		<a href="<?= tpl_link_call('node','invoke',$toModerate[$key]->nodeId,'setPublished') ?>"><?= tpl_text('Publish') ?></a> |
		<a href="<?= tpl_link_call('node','invoke',$toModerate[$key]->nodeId,'setRefused') ?>"><?= tpl_text('Refuse') ?></a>
	</div>
	<div class="indent bg-light">
		<? $this->render($toModerate[$key],'list_view.tpl') ?>
	</div>
	<br />
	<? } ?>
</div>


<div class="page_column_medium">
	<div class="page_text_header">Refused nodes</div>
	<? foreach (array_keys($refused) as $key) { ?>
	<div style="margin-bottom:3px;">
		<a href="<?= tpl_link_call('node','invoke',$refused[$key]->nodeId,'setModerated') ?>"><?= tpl_text('Reinsert in queue') ?></a>
	</div>
	<div class="indent bg-light">
		<? $this->render($refused[$key],'list_view.tpl') ?>
	</div>
	<br />
	<? } ?>
</div>