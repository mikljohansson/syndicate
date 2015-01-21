<? $issue = $node->getIssue(); $item = $node->getItem(); ?>
<a title="<?= tpl_text('Remove item from issue') ?>" 
	href="<?= tpl_link_call($issue->getHandler(),'invoke',$issue->nodeId,'removeItem',$item->nodeId) ?>"><img src="<?= tpl_design_uri('image/icon/close_small.gif') ?>" border="0" /></a>
<? $this->render($item,'head_view.tpl') ?><br />