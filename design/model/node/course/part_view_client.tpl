<? global $synd_user; ?>

<div class="indent">
	<? if ($client->getEmail() && !$synd_user->isNull()) { ?>
	Email: <a href="mailto:<?= $client->getEmail() ?>"><?= $client->getEmail() ?></a><br />
	<? } ?>
	<? if ($client->getPhone()) { ?>
	Phone: <?= $client->getPhone() ?><br />
	<? } ?>
</div>

<h3><?= tpl_text('Testresults for %s', $node->toString()) ?></h3>
<div class="list indent">
	<? if (count($attempts = $node->getClientAttempts($client))) { ?>
		<? $this->display('model/node/page/table.tpl',array('list'=>$attempts)) ?>
	<? } else { ?>
		<em><?= tpl_text('No testresults found') ?></em>
	<? } ?>
</div>

<ul class="Actions">
	<li><a href="javascript:synd_ole_call('<?= tpl_view_call('course','delete') ?>')" 
		title="<?= tpl_text('Delete the selected questions') 
		?>"><?= tpl_text('Delete selected') ?></a></li>
</ul>