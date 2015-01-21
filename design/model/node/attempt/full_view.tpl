<? if (count($questions = $node->getQuestions())) { ?>
	<? $this->display('model/node/question/table.tpl',array('list'=>$questions,'attempt'=>$node,'questionNumber'=>&$questionNumber)) ?>
<? } else { ?>
	<em><?= tpl_text('No questions in this attempt.') ?></em>
<? } ?>