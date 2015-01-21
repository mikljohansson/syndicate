<table style="width:100%; margin-bottom:10px;">
<tr>
	<? if (isset($questionNumber)) { ?>
	<td style="padding-right:5px; width:20px;"><b><?= $questionNumber++ ?>:</b></td>
	<? } ?>
	<td>
		<div class="Abstract"><?= $node->toString() ?></div>
		<table class="Body" style="width:500px;">
			<? foreach ($node->getOptions() as $option) { ?>
			<tr>
				<td><?= $option['INFO_OPTION'] ?></td>
				<td class="right" style="width:40px;">
					<?= round($node->getOptionAnswerFrequency($option['OPTION_NODE_ID'])*100) ?>%
				</td>
				<td class="right" style="width:40px; padding-right:2em;">
					<? if ($node->getOptionAnswerCount($option['OPTION_NODE_ID'])) { ?>
					<?= $node->getOptionAnswerCount($option['OPTION_NODE_ID']) ?>/<?= $node->getAnswerCount() ?>
					<? } ?>
				</td>
				<td style="width:100px;">
					<a href="<?= tpl_link_call($node->getHandler(),'invoke',
						$node->nodeId,'addStatisticsFilter',$option['OPTION_NODE_ID']) ?>"><?= 
					tpl_text('Filter') ?></a>
				</td>
			</tr>
			<? } ?>
		</table>
		<ul class="Actions">
			<li><a href="<?= rtrim(tpl_link($node->getHandler(),'invoke',
				$node->nodeId,'xls',$node->getExcelFilename($questionNumber)),'/') ?>"><?= 
				tpl_text('Statistics grouped by this question (Excel)') ?></a></li>
		</ul>
	</td>
</tr>
</table>