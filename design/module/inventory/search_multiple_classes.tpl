<? 
if (!empty($request['query'])) {
	$collections = array();
	$inventory = Module::getInstance('inventory');
	$query = $inventory->searchPreprocess($request['query']);

	$limit = 20;
	$this->assign('limit', $limit);

	foreach ($classes as $clsid) {
		if (null != ($collection = SyndLib::runHook('search', $clsid, $query))) {
			$collections[$clsid] = $collection;
			unset($collection);
		}
	}

	if (!empty($request['redirect']) && 1 == array_sum(SyndLib::invoke($collections,'getCount'))) {
		foreach (array_keys($collections) as $clsid) {
			if (1 == $collections[$clsid]->getCount()) {
				$matches = $collections[$clsid]->getContents(0,1);
				if ('lease' != $clsid)
					$mres['redirect'] = tpl_view($matches[key($matches)]->getHandler(),'view',$matches[key($matches)]->nodeId);
				else {
					$client = $matches[key($matches)]->getCustomer();
					$mres['redirect'] = tpl_view('user','summary',$client->nodeId);
				}
				return;
			}
		}
	}
}

?>
<form action="<?= tpl_link('inventory','search') ?>" method="get">
	<input type="text" name="query" value="<?= tpl_attribute($request['query']) ?>" style="width:400px;" />
	<input type="submit" name="post" value="<?= tpl_text('Search') ?>" />
	<select name="section" onchange="this.form.submit();">
		<?= tpl_form_options($sections, $section) ?>
	</select>
	<br />
</form>

<div class="Info" style="margin-top:1em;">
	<?= tpl_text('Use +/- to force include and exclude of words and &quot; to specify phrases.') ?>
	<? if (null != ($link = Module::runHook('search_help_uri'))) { ?>
	<?= tpl_translate('Consult this <a href="%s">page</a> for more information.', $link) ?>
	<? } ?>
</div>

<? if (isset($collections)) { ?>
	<? foreach (array_keys($collections) as $clsid) {
		$offset = isset($_REQUEST["offset_$clsid"]) ? $_REQUEST["offset_$clsid"] : 0;
		if (!count($matches = SyndLib::filter($collections[$clsid]->getContents($offset, $limit),'isPermitted','read')))
			continue;
		$found = true;
		?>
		<input type="hidden" name="collections[]" value="<?= $collections[$clsid]->id() ?>" />
		<div class="Result">
			<?= tpl_text("Results %d-%d of %d %s matching <b>'%s'</b>", $offset+1, 
				$offset+count($matches), $collections[$clsid]->getCount(), tpl_text($clsid.'s'), tpl_chop($request['query'],25)) ?>
			<? $this->display(tpl_design_path('gui/pager.tpl'),array(
				'offset' => $offset,'count' => $collections[$clsid]->getCount(),
				'offset_variable_name' => "offset_$clsid")) ?>
		</div>
		<?= tpl_gui_table($clsid,$matches,'view.tpl',array('collection'=>$collections[$clsid])) ?>
	<? } ?>
	<? if (empty($found)) { ?>
		<?= tpl_text("No results matching <b>'%s'</b> were found", $request['query']) ?>
	<? } ?>
<? } ?>
