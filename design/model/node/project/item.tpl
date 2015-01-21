<div class="Item">
	<h3><a href="<?= tpl_link('issue','project',$node->getProjectId()) ?>"><?= $node->toString() ?></a></h3>
	<? if (null != $node->getDescription()) { ?>
	<div class="Abstract">
		<?= tpl_html_links($node->getDescription()) ?>
	</div>
	<? } ?>
	<ul class="Actions">
		<li><a href="<?= tpl_link('issue','invoke',$node->nodeId,'newIssue') ?>"><?= 
			tpl_text('New issue for %s',$node->toString()) ?></a></li>
	</ul>
</div>
