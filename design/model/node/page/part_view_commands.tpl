<? global $synd_user; ?>
<li class="PrinterAction"><a href="<?= tpl_link($node->getHandler(),'view',$node->nodeId,'print') 
	?>" title="<?= tpl_text('Displays the content of the course book in a printer friendly format.') 
	?>"><?= tpl_text('Display the course book') ?></a></li>

<? if ($node->isPermitted('diagnostic_test')) { ?>
<li><a href="<?= tpl_link($node->getHandler(),'view',$node->nodeId,'diagnostic') 
	?>" title="<?= tpl_text('Take a diagnostic test to see what you need to study.') 
	?>"><?= tpl_text('Take the diagnostic test') ?></a></li>
<? } ?>

<? if ($node->isPermitted('progress_check')) { ?>
<li class="ProgressAction"><a href="<?= tpl_link($node->getHandler(),'view',$node->nodeId,'progress') 
	?>" title="<?= tpl_text('Check up on how your studies are going.') 
	?>"><?= tpl_text('Take a progress check') ?></a></li>
<? } ?>

<? if (count($node->getProgressAttempts($synd_user))) { ?>
<li class="ProgressAction"><a href="<?= tpl_link($node->getHandler(),'view',$node->nodeId,'results') 
	?>" title="<?= tpl_text('Show the results from your progress checks and diagnostic tests.') 
	?>"><?= tpl_text('My progress results') ?></a></li>
<? } ?>

<? if ($node->isPermitted('write')) { ?>
<li><a href="<?= tpl_link_call($node->getHandler(),'insert','page',
	array('data'=>array('PARENT_NODE_ID'=>$node->nodeId))) ?>" 
	title="<?= tpl_text('Creates a new chapter under the current page') 
	?>"><?= tpl_text('Create new page') ?></a></li>
<li class="QuestionAction"><a href="<?= tpl_link_call($node->getHandler(),'insert','questionnaire',
	array('data'=>array('PARENT_NODE_ID'=>$node->nodeId))) ?>" 
	title="<?= tpl_text('Add a new questionnaire under the current page') 
	?>"><?= tpl_text('Add questionnaire') ?></a></li>
<? 
/*
<li class="QuestionAction"><a href="<?= tpl_link_call($node->getHandler(),'insert','diagnostic',
	array('data'=>array('PARENT_NODE_ID'=>$node->nodeId))) ?>" 
	title="<?= tpl_text('Add a new diagnostic test under the current page') 
	?>"><?= tpl_text('Add diagnostic test') ?></a></li>

*/ 
?>
<li><a href="<?= tpl_link_call($node->getHandler(),'invoke',$node->nodeId,'newFile') 
	?>" title="<?= tpl_text('Upload a file to the current page') 
	?>"><?= tpl_text('Upload file under this page') ?></a></li>
<? } ?>
