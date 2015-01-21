<? if (strlen($accesskey)) { ?>
<a href="<?= $this->quote($node->getLocation()) ?>" accesskey="<?= $this->quote($accesskey) ?>" title="<?= $this->text('Accesskey: %s',$accesskey) ?>"><?= $this->quote($name) ?></a>
<? } else { ?>
<a href="<?= $this->quote($node->getLocation()) ?>"><?= $this->quote($name) ?></a>
<? } ?>