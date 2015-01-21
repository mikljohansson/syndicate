			<h3><a href="<?= $document['URI'] ?>"><img src="<?= tpl_design_uri('image/icon/16x16/xls.gif') ?>" alt="[<?= $ext ?>]" height="16" width="16" /> <?= htmlspecialchars($document['TITLE']) ?></a></h3>
			<div class="Info"><?= strtoupper($ext) ?>/Microsoft Excel <? $this->render($node,'modified.tpl') ?></div>
