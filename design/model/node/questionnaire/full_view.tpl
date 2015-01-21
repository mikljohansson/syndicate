<? tpl_load_script(tpl_design_uri('js/ole.js')) ?>
<div class="Article">
	<? $this->render($node,'part_view_header.tpl') ?>

	<br />
	<div class="Body">
		<? $content = $node->getContent(); print $content->toString(); ?>
	</div>

	<br />
	<hr />
	<? if (count($questions = $node->getQuestions())) { ?>
		<? $attempt = $node->getTrainingAttempt(); ?>
	<form method="post">
		<div>
			<? $this->display('model/node/question/table.tpl',array('list'=>$questions,'attempt'=>$attempt)) ?>
		</div>
		<div style="margin:15px;">
			<input type="submit" value="<?= tpl_text('Check my answers') ?>" />
			<? if ($attempt->hasAnswers()) { ?>
			<input type="button" value="<?= tpl_text('Take the exercise again') ?>" onclick="window.location=window.location;" />
			<? } ?>
		</div>
	</form>
	<? } else { ?>
		<? if ($node->isPermitted('write')) { ?>
			<em><?= tpl_text('Use the links below to add questions.') ?></em>
		<? } else { ?>
			<em><?= tpl_text('No questions available.') ?></em>
		<? } ?>
	<? } ?>
	
	<? if ($node->isPermitted('write')) { ?>
	<table style="width:100%;">
	<tr>
		<td class="top" style="padding:10px;">
			<ul class="Actions">
				<li><a href="<?= tpl_link_call($node->getHandler(),'insert','open_ended_text',
					array('data'=>array('PARENT_NODE_ID'=>$node->nodeId))) ?>" 
					title="<?= tpl_text('Add a new Fill-in-the-Blank question to the current questionnaire') ?>"><?= tpl_text("Add 'Fill-in-the-Blank' question") ?></a></li>
				<li><a href="<?= tpl_link_call($node->getHandler(),'insert','multiple_choice',
					array('data'=>array('PARENT_NODE_ID'=>$node->nodeId))) ?>" 
					title="<?= tpl_text('Add a new Multiple-Choice question to the current questionnaire') ?>"><?= tpl_text("Add 'Multiple-Choice' question") ?></a></li>
			</ul>
		</td>
		<td class="top" style="padding:10px;">
			<ul class="Actions">
				<li><a href="javascript:synd_ole_call('<?= tpl_view_call('course','cut') ?>')" 
					title="<?= tpl_text('Cut the selected questions from this questionnaire') 
					?>"><?= tpl_text('Cut selected') ?></a></li>
				<li><a href="javascript:synd_ole_call('<?= tpl_view_call('course','copy') ?>')" 
					title="<?= tpl_text('Copy the selected questions') 
					?>"><?= tpl_text('Copy selected') ?></a></li>
				<li><a href="javascript:synd_ole_call('<?= tpl_view_call('course','delete') ?>')" 
					title="<?= tpl_text('Delete the selected questions') 
					?>"><?= tpl_text('Delete selected') ?></a></li>
			</ul>
		</td>
	</tr>
	</table>
	<? } ?>

	<? $this->render($node,'part_view_footer.tpl') ?>
</div>