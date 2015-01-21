<? 
if (0 === strpos($section,'class.')) {
	$class = SyndNodeLib::getInstance($section);
	$fields = $class->getNumericFields();
}

if (!empty($request['query']) || isset($request['post'])) {
	$limit = 50;
	$offset = isset($_REQUEST['offset']) ? $_REQUEST['offset'] : 0;
	$inventory = Module::getInstance('inventory');
	$collection = $inventory->_search($request['query'], $offset, $limit, null, null, '||');

	$this->assign('limit', $limit);
	$this->assign('offset', $offset);
	$this->assign('count', $count = $collection->getCount());
}

?>
<form action="<?= tpl_link('inventory','search','specific') ?>" method="post">
	<?= tpl_form_textarea('query',$request['query'],array('cols'=>60)) ?>
	<input type="submit" name="post" value="<?= tpl_text('Search') ?>" />
</form>
<? if (isset($collection)) { ?>
	<input type="hidden" name="collections[]" value="<?= $collection->id() ?>" />
	<? if (count($matches = $collection->getContents($offset, $limit))) { ?>
		<div class="Result">
			<?= tpl_text("Results %d-%d of %d matching <b>'%s'</b>", $offset+1, 
				$offset+count($matches), $count, tpl_chop($request['query'],25)) ?>
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