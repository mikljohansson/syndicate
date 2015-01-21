<?= $this->quote($node->getDescription()) ?>
<b><?= $this->text('Relative to ') ?></b><?= $node->getRelativeToday() ? $this->text('today') : $this->text('date in issue') ?>