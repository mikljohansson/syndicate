<div class="Task">
	<h5>
		<? if ($node->isPermitted('admin')) { ?>
			<? if ($node->data['FLAG_PROTECTED']) { ?>
			<a href="<?= tpl_link_call($node->getHandler(),'invoke',$node->nodeId,'setProtected',0) ?>"><img src="<?= tpl_design_uri('image/icon/16x16/locked.gif') 
				?>" width="16" height="16" alt="<?= tpl_text('Show') ?>" title="<?= tpl_text('This comment is hidden from the customer') ?>" /></a>
			<? } else { ?>
			<a href="<?= tpl_link_call($node->getHandler(),'invoke',$node->nodeId,'setProtected',1) ?>"><img src="<?= tpl_design_uri('image/icon/16x16/unlocked.gif') 
				?>" width="16" height="16" alt="<?= tpl_text('Hide') ?>" title="<?= tpl_text('This comment is visible to the customer') ?>" /></a>
			<? } ?>
		<? } ?>
		<? if ($node->getParent()->isPermitted('write')) { ?>
		<a href="<?= tpl_link_call($node->getHandler(),'invoke',$node->nodeId,'forward') ?>"><img src="<?= tpl_design_uri('image/icon/16x16/forward.gif') 
			?>" width="16" height="16" alt="<?= tpl_text('Forward') ?>" title="<?= tpl_text('Forward this message') ?>" /></a>
		<a href="<?= tpl_link_call($node->getHandler(),'invoke',$node->nodeId,'reply') ?>"><img src="<?= tpl_design_uri('image/icon/16x16/reply.gif') 
			?>" width="16" height="16" alt="<?= tpl_text('Reply') ?>" title="<?= tpl_text('Reply to this message') ?>" /></a>
		<? $this->render($node->getContent(),'icons.tpl',null,false) ?>
		<? } ?>
		<a href="<?= tpl_link('user','summary',$node->getCreator()->nodeId) ?>"><?= $node->getCreator()->toString() ?></a>
		<span class="Info">(<?
			print ucwords(tpl_strftime('%A, %d %b %Y %H:%M',$node->data['TS_CREATE']));
			if ($node->getDuration()) 
				print ', '.tpl_duration($node->getDuration()); 
		?>)</span>
	</h5>
	<? $this->render($node->getContent(),'full_view.tpl',array('filter' => array($node->getParent(), '_callback_filter'))) ?>
	<? $this->render($node->getParent(),$edit?'attachment/edit.tpl':'attachment/view.tpl',array('node'=>$node)) ?>
</div>