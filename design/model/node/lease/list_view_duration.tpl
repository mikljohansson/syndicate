<? if ($node->data['TS_CREATE']) { ?>
	<?= tpl_text('Created on %s', ucwords(tpl_strftime('%d %b %Y', $node->data['TS_CREATE']))) ?>,
<? } ?>

<? if ($node->isTerminated()) { ?>
	<?= tpl_text('Terminated on %s', ucwords(tpl_strftime('%d %b %Y', $node->data['TS_TERMINATED']))) ?>
<? } else if ($node->data['TS_EXPIRE']) { ?>
	<?= tpl_text('Expires %s', ucwords(tpl_strftime('%d %b %Y', $node->data['TS_EXPIRE']))) ?>
<? } else { ?>
	<?= tpl_text('No expiry date set') ?>
<? } ?>
