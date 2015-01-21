<? 
if (0 === strpos($section,'class.')) {
	$class = SyndNodeLib::getInstance($section);
	$fields = $class->getNumericFields();
}

if (!empty($request['query']) || isset($request['post'])) {
	$limit = 50;
	$offset = isset($_REQUEST['offset']) ? $_REQUEST['offset'] : 0;
	$inventory = Module::getInstance('inventory');
	$collection = $inventory->_search($request['query'], $offset, $limit, 
		isset($class) ? $class->nodeId : null, array_filter((array)$request['fields']));

	$this->assign('limit', $limit);
	$this->assign('offset', $offset);
	$this->assign('count', $count = $collection->getCount());
}

?>
<form action="<?= tpl_link('inventory','search') ?>" method="get">
	<input type="text" name="query" value="<?= tpl_attribute($request['query']) ?>" style="width:400px;" />
	<input type="submit" name="post" value="<?= tpl_text('Search') ?>" />

	<select name="section" onchange="this.form.submit();">
		<?= tpl_form_options($sections, $section) ?>
	</select><br />

	<div class="Info" style="margin-top:1em;">
		<?= tpl_text('The wildcards * and ? are supported. Use +/- to include/exclude keywords and " to specify phrases.') ?>
		<? if (!empty($fields)) { ?>
		<?= tpl_text("Boolean operators (<,>,<=,>=,!=,&,|) and parantheses together with numerals are supported. For example '>= 256 & < 512'.") ?>
		<? } else if (!isset($fields)) { ?>
		<?= tpl_text('Select a category for more search options') ?>
		<? } ?>
	</div>

	<? if (!empty($fields)) { ?>
	<table class="Block">
		<? foreach ($fields as $id => $field) { ?>
		<tr class="<?= tpl_cycle(array('odd','even')) ?>">
			<th style="width:150px;"><?= $field ?></th>
			<td><input type="text" name="fields[<?= $id ?>]" value="<?= tpl_value($request['fields'][$id]) ?>" size="50" /></td>
		</tr>
		<? } ?>
	</table>
	<? } ?>
</form>
<? if (isset($collection)) { ?>
	<input type="hidden" name="collections[]" value="<?= $collection->id() ?>" />
	<? if (count($matches = $collection->getContents($offset, $limit))) { ?>
		<div class="Result">
			<?= tpl_text("Results %d-%d of %d matching <b>'%s'</b>", $offset+1, 
				$offset+count($matches), $count, $request['query']) ?>
			<? $this->display(tpl_design_path('gui/pager.tpl')) ?>
		</div>
		<?= tpl_gui_table('item',SyndLib::filter($matches,'isPermitted','read'),'view_result.tpl') ?>
		<? $this->display(tpl_design_path('gui/pager.tpl')) ?>
	<? } else { ?>
		<div class="Result">
			<?= tpl_text("No results matching <b>'%s'</b> were found", $request['query']) ?>
		</div>
	<? } ?>
<? } ?>
