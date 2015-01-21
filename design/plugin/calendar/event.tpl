<?= trim($event->getSummary()) ?>
<? if ($writable) { ?>
<ul class="Actions">
	<li><a href="<?= tpl_link_call('issue','calendar','deleteEvent',$issue->nodeId,$key,$id) ?>"><?= tpl_text('Delete') ?></a></li>
</ul>
<? } ?>