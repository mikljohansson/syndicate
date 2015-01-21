<? $creator = SyndNodeLib::getInstance($node->data['CREATE_NODE_ID']); $content = $node->getContent(); ?>
<p class="Task">
	<a href="<?= tpl_link('user','summary',$creator->nodeId) ?>"><?= $creator->toString() ?></a>
	<span class="Info">(<?
		print ucwords(tpl_strftime('%Y-%m-%d %H:%M',$node->data['TS_CREATE']));
		if ($node->getDuration()) 
			print ', '.tpl_duration($node->getDuration(),null,'h','m');
	?>)</span><br />
	<?= tpl_html_format($content->toString()) ?>
</p>