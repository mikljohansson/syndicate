<form method="get">
	<input type="text" name="query" value="<?= tpl_value($request['query']) ?>" style="width:400px;" size="80" />
	<input type="submit" class="button"value="<?= tpl_text('Search') ?>" /><br />
	<div style="margin-top:5px;">
		<?= tpl_form_radiobutton('section',$section,null) ?> <?= tpl_text('All') ?>
		<?= tpl_form_radiobutton('section',$section,'node.user') ?> <?= tpl_text('Users') ?>
		<?= tpl_form_radiobutton('section',$section,'node.text') ?> <?= tpl_text('Texts/Blogs') ?>
		<?= tpl_form_radiobutton('section',$section,'node.list.album,node.file') ?> <?= tpl_text('Albums/Media') ?>
	</div>
</form>

<? if (isset($expandSet) && count($expandSet)) { ?>
<div style="margin-top:8px;">
	<h3><?= tpl_text('Related seaches') ?></h3>
	<div class="indent">
		<? foreach ($expandSet as $term) { ?>
		<a href="?query=<?= $term ?>"><?= trim($term) ?></a>&nbsp;&nbsp;
		<? } ?>
	</div>
</div>
<? } ?>

<div style="margin-top:10px">
<? if (isset($matchSet)) { ?>
	<div>
	<? if (count($matchSet)) { ?>
		<?= tpl_text("Results %d-%d of %d matching <b>'%s'</b>", $offset+1, $offset+count($matchSet), $count, $request['query']) ?>
		<? $this->display(tpl_design_path('gui/pager.tpl')) ?>

		<div class="list" style="margin-top:8px;">
			<? $this->iterate($matchSet,'list_view.tpl',array('highlight'=>$highlight)) ?>
		</div>

		<? $this->display(tpl_design_path('gui/pager.tpl')) ?>
	<? } else { ?>
		<em><?= tpl_text("No results were found containing <b>'%s'</b>", $request['query']) ?></em>
	<? } ?>
	</div>
<? } ?>
</div>