<? $file = $node->getFile(); ?>
<div class="Article">
	<div class="Header">
		<h2><?= $node->toString() ?></h2>
		<div class="Info">
			<?= tpl_translate('Posted by %s on %s', 
				$this->fetchnode($node->getCustomer(),'head_view.tpl'), 
				tpl_quote(ucwords(tpl_strftime('%A, %d %B %Y', $node->data['TS_CREATE'])))) ?>; 
			<?= ceil($file->getSize()/1024) ?>Kb<? 
			if ($node->isPermitted('write')) { ?>;
			<a href="<?= tpl_link_call($node->getHandler(),'edit',$node->nodeId) ?>"><?= tpl_text('Edit this file') ?></a>
			<? } ?>
		</div>
	</div>
	<ul class="Actions">
		<li><a href="<?= $file->uri() ?>"  target="_blank"><?= tpl_text("Download the file <em>'%s'</em>", $file->toString()) ?></a></li>
	</ul>
	<? if ($node->hasImage()) { $image = $node->getImage(); ?>
	<div class="Image">
		<a href="<?= $file->uri() ?>" alt="<?= tpl_text('Download') ?>" target="_blank"><img src="<?= $image->getBoxedUri(640,480) ?>" /></a>
	</div>
	<? } ?>
	<? if (null != $node->getDescription()) { ?>
	<div class="Abstract">
		<?= $node->getDescription() ?>
	</div>
	<? } ?>
</div>
