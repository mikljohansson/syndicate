<? $eventLogger = $node->getEventLogger(); ?>
<? if (count($eventLogger->getEvents())) { ?>
	<? $this->iterate($eventLogger->getEvents(),'full_view.tpl'); ?>
<? } else { ?>
	<em><?= $this->text('No history found') ?></em>
<? } ?>
