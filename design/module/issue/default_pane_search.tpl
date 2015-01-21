<?

if (isset($request['q'])) {
	$module->_title .= ': '.tpl_chop(strip_tags($request['q']), 15);
	$collection = SyndLib::runHook('search', 'issue', $request['q'], $request['o'], true);

	if (null != $collection) {
		$limit = 25;
		$offset = isset($request['offset']) ? $request['offset'] : 0;
		$matches = SyndLib::filter($collection->getContents($offset, $limit), 'isPermitted', 'read');
		$count = $collection->getCount();

		if (null != $request['q'] && !isset($_SESSION['synd']['issue']['search'][$request['q']])) {
			$_SESSION['synd']['issue']['search'][$request['q']] = $request;
			if (count($_SESSION['synd']['issue']['search']) > 4)
				array_shift($_SESSION['synd']['issue']['search']);
		}
		
		if ($count < 10)
			$this->assign('suggestions', SyndLib::runHook('spelling', $request['q']));
	}
}

?>
<h2><?= tpl_text('Search for issues') ?></h2>
<form action="<?= tpl_link('issue','search') ?>" method="get">
	<input type="text" name="q" id="IssueSearch" value="<?= tpl_value($request['q']) ?>" size="80" />
	<select name="o" onchange="this.form.submit();">
		<?= tpl_form_options(array('date'=>tpl_text('Order by date'),'relevance'=>tpl_text('Order by relevance')), $request['o']) ?>
	</select>
	<input type="submit" value="<?= tpl_text('Search') ?>" />
</form>

<div style="margin-top:1em;">
<? if (isset($matches)) { ?>
	<? if ($count && ($offset || !empty($matches))) { ?>
		<input type="hidden" name="collections[]" value="<?= $collection->id() ?>" />
		<div class="Result">
			<?
			print tpl_text("Results %d-%d of about %d matching <b>'%s'</b>", 
				$offset+1, $offset+count($matches), $count, $request['q']);
			if (min($count - $offset, $limit) > count($matches)) 
				print ' <span class="Info">'.tpl_text('(Some matches are excluded due to access restrictions)').'</span>';
			?><br />
			<? $this->display(tpl_design_path('gui/pager.tpl'),
				array('limit'=>$limit,'offset'=>$offset,'count'=>$count)) ?>
		</div>
		<? $this->display('module/issue/default_query_suggestions.tpl'); ?>
		<? $this->display('model/node/issue/table.tpl',array('list'=>$matches,'hideSorting'=>true)) ?>
	<? } else { ?>
		<? $this->display(tpl_design_path('module/issue/default_query_suggestions.tpl')); ?>
		<em><?= tpl_text("No results matching <b>'%s'</b> were found", $request['q']) ?></em>
	<? } ?>
<? } else { ?>
	<div class="Info">
		<?= tpl_text('Use +/- to force include and exclude of words and &quot; to specify phrases.') ?>
		<? if (null != ($link = Module::runHook('search_help_uri'))) { ?>
		<?= tpl_translate('Consult this <a href="%s">page</a> for more information.', $link) ?>
		<? } ?>
	</div>
<? } ?>
</div>
