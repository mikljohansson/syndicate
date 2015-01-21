<ul class="Actions">
	<li><a href="<?= tpl_link_call('issue','workflow','newWorkflow',$project->nodeId) ?>"><?= $this->text('Create workflow') ?></a></li>
</ul>

<? if (($count = count($workflows))) { ?>
	<div class="Result">
		<?= tpl_text("Results %d-%d of %d workflows", $offset+1, $offset+min($count-$offset,$limit), $count); ?>
		<? $this->display('gui/pager.tpl',array('offset'=>$offset,'limit'=>$limit,'count'=>$count)) ?>
	</div>
	<? $this->display('model/node/workflow/table.tpl',array('list'=>$workflows->getIterator($offset,$limit,$order))) ?>
<? } else { ?>
	<div class="Result">
		<?= tpl_text("No workflows were found") ?>
	</div>
<? } ?>
