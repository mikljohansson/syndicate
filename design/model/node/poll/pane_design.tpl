	<div class="Body" style="margin-top:10px; margin-bottom:10px;">
		<? $content = $node->getContent(); print $content->toString(); ?>

		<? if ($node->isPermitted('write')) { ?>
		<ul class="Actions">
			<li><a href="<?= tpl_link_call($node->getHandler(),'edit',$node->nodeId) ?>">
				<?= tpl_text('Edit this text') ?></a></li>
		</ul>
		<? } ?>

		<hr />
		<? $this->iterate($node->getChildren(),'item_design.tpl',
			array('attempt' => $node->createTrainingAttempt(), 'questionNumber' => 1)) ?>
	</div>

	<? if ($node->isPermitted('write')) { ?>
		<? $page = $node->getPage(); ?>
		<table width="100%">
		<tr>
			<td class="top" style="width:250px; padding:10px;">
				<ul class="Actions">
					<li><a href="<?= tpl_link_call($node->getHandler(),'insert','questionnaire',
						array('data'=>array('PARENT_NODE_ID'=>$page->nodeId))) ?>" 
						title="<?= tpl_text('Add a new chapter to the current poll') 
						?>"><?= tpl_text('Add chapter') ?></a></li>
					<li><a href="<?= tpl_link_call($node->getHandler(),'insert','open_ended_text',
						array('data'=>array('PARENT_NODE_ID' => $page->nodeId))) ?>">
						<?= tpl_text('Add Fill-in-the-Blank question') ?></a></li>
					<li><a href="<?= tpl_link_call($node->getHandler(),'insert','multiple_choice',
						array('data'=>array('PARENT_NODE_ID' => $page->nodeId))) ?>">
						<?= tpl_text('Add Multiple-Choice question') ?></a></li>
				</ul>
			</td>
			<td class="top" style="padding:10px;">
				<ul class="Actions">
					<li><a href="javascript:synd_ole_call('<?= tpl_view_call($node->getHandler(),'cut',$page->nodeId) ?>')" 
						title="<?= tpl_text('Cut the selected questions from this questionnaire') 
						?>"><?= tpl_text('Cut selected') ?></a></li>
					<li><a href="javascript:synd_ole_call('<?= tpl_view_call($node->getHandler(),'copy',$page->nodeId) ?>')" 
						title="<?= tpl_text('Copy the selected questions') 
						?>"><?= tpl_text('Copy selected') ?></a></li>
					<li><a href="javascript:synd_ole_call('<?= tpl_view_call($node->getHandler(),'delete') ?>')" 
						title="<?= tpl_text('Delete the selected questions') 
						?>"><?= tpl_text('Delete selected') ?></a></li>
				</ul>
			</td>
		</table>
	<? } ?>
