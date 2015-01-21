<div class="SearchResult">
	<h3><? $this->render($user,'contact.tpl') ?></h3>
	<? if (count($items = SyndLib::runHook('user_summary_result', $request, $this, $user))) { ?>
	<ul class="Actions">
		<? foreach ($items as $item) { ?>
		<li><?= $item ?></li>
		<? } ?>
	</ul>
	<? } ?>
	<hr />
</div>