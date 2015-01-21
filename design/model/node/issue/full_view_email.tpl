<? 
if (!isset($request['event'], $request['listeners']))
	throw new InvalidArgumentException();

$logger = $node->getEventLogger();
$listeners = $request['listeners'];
if (null == ($event = SyndLib::getInstance($request['event'])) ||
	null == ($listener = SyndLib::getInstance(array_shift($listeners))))
	throw new NotFoundException();

?>
<div class="Issue">
	<input type="hidden" name="selection[]" value="<?= $node->id() ?>" />
	<h3><?= $this->text('Email sent to %s <span class="Info">(%s)</span> on %s', 
		$listener->toString(), $listener->getEmail(), ucwords(tpl_strftime('%A, %d %B %Y %R', $event->getTimestamp()))) ?></h3>
	<div class="Email">
		<div class="EmailHeader">
			<?= $this->text('From: %s', implode(', ', $node->getEmailSenders())) ?><br />
			<?= $this->text('To: %s', $listener->getEmail()) ?><br />
			<? if (!empty($listeners)) { ?>
			<?= $this->text('Cc: %s', implode(', ', SyndLib::invoke(SyndLib::getInstances($listeners),'getEmail'))) ?><br />
			<? } ?>
			<?= $this->text('Subject: %s', $node->getEmailSubject()) ?>
		</div>
		<?= $this->format($node->getEmailBody($event, array())); ?>
	</div>
</div>
