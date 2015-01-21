<div class="Article">
	<div class="Header">
		<h2><?= $node->toString() ?></h2>
	</div>
	<? $this->render($node->getParent(),'filters.tpl') ?>
	
	<dl class="Actions">
		<dt><a href="<?= rtrim(tpl_link($node->getHandler(),'invoke',
			$node->nodeId,'xls',$node->getExcelFilename($request['number'])),'/') ?>"><?= tpl_text('Excel version') ?></a></dt>
		<dd><?= tpl_text('Download all the replies as a simple Excel spreadsheet.') ?></dd>
	</dl>

	<h3><?= tpl_text('Answers to this question') ?></h3><hr />
	<? foreach ($node->getAnswers() as $answer) { ?>
	<div class="Item">
		<? $this->render(SyndNodeLib::getInstance($answer['ATTEMPT_NODE_ID']),'info.tpl') ?>
		<?= $answer['INFO_ANSWER'] ?>
	</div>
	<? } ?>
</div>