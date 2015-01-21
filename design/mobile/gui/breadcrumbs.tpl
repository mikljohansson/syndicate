<div>
	<a href="/"><?= tpl_text('Home') ?></a>
	<? foreach ($crumbs = (array)SyndLib::runHook('breadcrumbs', $this) as $crumb) { ?>
	&raquo; <a href="<?= $crumb['uri'] ?>"><?= $crumb['text'] ?></a>
	<? } ?>
</div>