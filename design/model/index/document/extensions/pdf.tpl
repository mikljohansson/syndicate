			<h3><a href="<?= $document['URI'] ?>"><img src="<?= tpl_design_uri('image/icon/16x16/pdf.gif') ?>" alt="[<?= $ext ?>]" height="16" width="16" /> <?= synd_htmlspecialchars($document['TITLE']) ?></a></h3>
			<div class="Info"><?= strtoupper($ext) ?>/Adobe Acrobat <? $this->render($node,'modified.tpl') ?></div>
