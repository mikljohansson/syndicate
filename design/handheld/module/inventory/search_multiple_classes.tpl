<h1><?= tpl_text('Search the inventory') ?></h1>

<form action="<?= tpl_view('inventory','search') ?>" method="get">
	<input type="text" name="query" value="<?= tpl_attribute($request['query']) ?>" size="12" />
	<input type="submit" name="post" value="<?= tpl_text('Search') ?>" />
</form>

<? if (isset($collections)) { ?>
<br />
<div>
	<? if (array_sum(SyndLib::invoke($collections,'getCount'))) { ?>
		<? foreach (array_keys($collections) as $classId) {
			if (!$collections[$classId]->getCount())
				continue;
			$offset = isset($_REQUEST["offset_$classId"]) ? $_REQUEST["offset_$classId"] : 0;
			$matches = $collections[$classId]->getContents($offset, $limit);
			?>
			<div class="Result">
				<?= tpl_text("Results %d-%d of %d %ss matching <b>'%s'</b>", $offset+1, 
					$offset+count($matches), $collections[$classId]->getCount(), $classId, $request['query']) ?>
				<? $this->display(tpl_design_path('gui/pager.tpl'),array(
					'offset' => $offset,'count' => $collections[$classId]->getCount(),
					'offset_variable_name' => "offset_$classId")) ?>
			</div>
			<div style="margin-bottom:2em;">
				<?= tpl_gui_table($classId,$matches,'view.tpl') ?>
			</div>
		<? } ?>
	<? } else { ?>
		<?= tpl_text("No results matching <b>'%s'</b> were found", $request['query']) ?>
	<? } ?>
</div>
<? } ?>