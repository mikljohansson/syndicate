<table style="width:100%; margin-bottom:10px;">
<tr>
	<? if (isset($questionNumber)) { ?>
	<td style="padding-right:5px; width:20px;"><b><?= $questionNumber ?>:</b></td>
	<? } ?>
	<td>
		<div class="Item">
			<div class="Abstract"><?= $node->toString() ?></div>
			<div class="Info">
				<?= tpl_text('%d replies to this question', $node->getAnswerCount()) ?>
			</div>
		</div>
		<? if ($node->isPermitted('statistics')) { ?>
		<ul class="Actions">
			<li><a href="<?= tpl_link($node->getHandler(),'view',$node->nodeId,'answers',array('number'=>$questionNumber)) ?>"><?= 
				tpl_text('Display answers') ?></a></li>
			<li><a href="<?= rtrim(tpl_link($node->getHandler(),'invoke',
				$node->nodeId,'xls',$node->getExcelFilename($questionNumber)),'/') ?>"><?= 
				tpl_text('Download answers (Excel)') ?></a></li>
		</ul>
		<? } ?>
	</td>
</tr>
</table>
<? if (isset($questionNumber)) $questionNumber++; ?>
