<? $creator = $node->getCreator(); ?>
<div>
	<h5>
		<a href="<?= tpl_link('user','summary',$creator->nodeId) ?>"><?= $creator->toString() ?></a>
		<span class="Info">(<?
			print ucwords(tpl_strftime('%A, %d %b %Y %H:%M',$node->data['TS_CREATE']));
			if ($node->getDuration()) 
				print ', '.tpl_duration($node->getDuration()); 
		?>)</span>
	</h5>
	<p><?= tpl_html_format($node->getDescription()) ?></p>
</div>