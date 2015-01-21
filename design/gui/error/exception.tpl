<? if (isset($exception)) { ?>
	<h3><?= tpl_text('Message') ?></h3>
	<pre><?= $exception->getMessage() ?><? if ($exception->getCode()) { ?> (<?= $exception->getCode() ?>)<? } ?></pre>
	<h3><?= tpl_text('Stacktrace') ?></h3>
	<pre><?= $exception->getTraceAsString() ?></pre>
	<? if (method_exists($exception, 'getDescription')) { ?>
	<h3><?= tpl_text('Description') ?></h3>
	<pre><?= $exception->getDescription() ?></pre>
	<? } ?>
<? } ?>