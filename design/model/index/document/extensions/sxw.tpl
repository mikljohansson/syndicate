			<h3><a href="<?= $document['URI'] ?>"><img src="<?= tpl_design_uri('image/icon/16x16/sxw.gif') ?>" alt="[<?= $ext ?>]" height="16" width="16" /> <?= htmlspecialchars($document['TITLE']) ?></a></h3>
			<div class="Info"><?= strtoupper($ext) ?>/OpenOffice.org <?= tpl_text('(%dKb, last modifed %s)', $node->getSize()/1024, date('Y-m-d', $node->getModificationTime()))) ?></div>
