<div style="margin-top:8px">
	<input type="hidden" name="collections[]" value="<?= $report->getCollection()->id() ?>" />
	<? if ($count = $report->getCount()) { ?>
		<? 
		$limit = 50;
		$offset = isset($_REQUEST['offset']) ? $_REQUEST['offset'] : 0;
		$contents = $report->getContents($offset, $limit); ?>
		<div class="Result">
			<?= tpl_text("Results %d-%d of %d", $offset+1, $offset+count($contents), $count) ?><br />
			<? $this->display(tpl_design_path('gui/pager.tpl'),
				array('limit'=>$limit,'offset'=>$offset,'count'=>$count)) ?>
		</div>
		<?= tpl_gui_table('lease',$contents,'view.tpl') ?>
		<? if ($count > $limit) { ?>
		<div class="Result">
			<? $this->display(tpl_design_path('gui/pager.tpl'),
				array('limit'=>$limit,'offset'=>$offset,'count'=>$count)) ?>
		</div>
		<? } ?>
	<? } else { ?>
		<div class="Notice">
			<?= tpl_text("No results were found") ?>
		</div>
	<? } ?>
</div>
