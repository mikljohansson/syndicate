<h4><?= tpl_text('Scheduled events') ?></h4>
<? foreach ($events as $event) { ?>
	<?= ucwords(strftime('%A, %d %B %Y %H:%M', $event->getStart())) ?> - <?= strftime('%H:%M', $event->getEnd()) ?>
	<div style="margin-left:1em;">
		<? if (null != ($location = $event->getLocation())) { ?>(<?= $location ?>)<? } ?>
		<? if (isset($eventids[$id = $event->getProperty('UID')])) { ?>
			<? foreach (SyndLib::getInstances($eventids[$id]) as $user) { ?>
			<? $this->render($user,'contact.tpl') ?><br />
			<? } ?>
		<? } ?>
	</div>
<? } ?>