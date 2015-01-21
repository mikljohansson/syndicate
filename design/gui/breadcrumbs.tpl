<? if (null != ($crumbs = SyndLib::runHook('breadcrumbs', $this))) { ?>
<div class="Breadcrumbs">
	<? foreach ($crumbs as $crumb) { ?>
	<? if ($i++) print '&raquo;'; ?> <?= isset($crumb['uri']) ? "<a href=\"{$crumb['uri']}\">{$crumb['text']}</a>" : $crumb['text'] ?>
	<? } ?>
</div>
<? } ?>
