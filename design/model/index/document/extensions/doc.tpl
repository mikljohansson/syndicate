			<h3><a href="<?= $node->getLocation() ?>"><img src="<?= tpl_design_uri('image/icon/16x16/doc.gif') ?>" alt="[<?= $ext ?>]" height="16" width="16" /> <?= htmlspecialchars($node->getTitle()) ?></a></h3>
			<div class="Info"><?= strtoupper($ext) ?>/Microsoft Word <? $this->render($node,'modified.tpl') ?></div>
