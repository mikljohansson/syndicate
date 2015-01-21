<? global $synd_user; ?>
<div class="Item">
	<div class="Header">
		<h3><a href="<?= tpl_link($node->getHandler(),'view',$node->getCourseId()) ?>"><?= $node->getTitle() ?></a></h3>
	</div>
	<? if (null != $node->getDescription()) { ?>
	<div class="Abstract">
		<?= tpl_html_format($node->getDescription()) ?>
	</div>
	<? } ?>
	<ul class="Actions">
		<? if ($node->isPermitted('register') && !$node->isMember($synd_user)) { ?>
		<li><a href="<?= tpl_link($node->getHandler(),'invoke',$node->nodeId,'register') ?>"><?= 
		tpl_text('Register to %s', $node->toString()) ?></a></li>
		<? } else if ($node->isMember($synd_user)) { ?>
		<li><a href="<?= tpl_link_call($node->getHandler(),'invoke',$node->nodeId,'unregister') ?>"><?= 
		tpl_text('Unregister from %s', $node->toString()) ?></a></li>
		<? } ?>
	</ul>
</div>