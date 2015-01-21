<div class="Item">
	<div class="Header">
		<? if ($node->isPermitted('read')) { ?>
		<h3><a href="<?= tpl_link($node->getHandler(),'project',$node->getProjectId()) ?>"><?= $node->toString() ?></a></h3>
		<? } else { ?>
		<h3><?= $node->toString() ?></h3>
		<? } ?>
	</div>
	<? if (null != $node->getDescription()) { ?>
	<div class="Abstract">
		<?= tpl_html_links($node->getDescription()) ?>
	</div>
	<? } ?>
</div>