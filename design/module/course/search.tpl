<h3><?= tpl_text('Search') ?></h3>

<? if (!is_array($request['filter'])) { ?>
<form action="<?= tpl_view('course','search') ?>" method="get">
	<input type="text" name="query" value="<?= tpl_value($request['query']) ?>" style="width:400px;" size="80" />
	<input type="submit" class="button"value="<?= tpl_text('Search') ?>" />
	<br />
	<div style="margin-top:5px;">
		<?= tpl_form_radiobutton('section',$section,'page,question','all') ?> 
			<label for="all"><?= tpl_text('All') ?></label>
		<?= tpl_form_radiobutton('section',$section,'question','questions') ?> 
			<label for="questions"><?= tpl_text('Questions') ?></label>
		<?= tpl_form_radiobutton('section',$section,'page','pages') ?> 
			<label for="pages"><?= tpl_text('Pages') ?></label>
	</div>
</form>
<? } else { ?>
<form method="get">
	<? foreach ((array)$request['filter'] as $key => $value) { ?>
	<input type="hidden" name="filter[<?= $key ?>]" value="<?= $value ?>" />
	<? } ?>
	<input type="text" name="query" value="<?= tpl_value($request['query']) ?>" style="width:400px;" size="80" />
	<input type="submit" class="button"value="<?= tpl_text('Search') ?>" />
</form>
<? } ?>

<? if (isset($suggestedQuery)) { ?>
<div style="margin:8px 0 0px 5px;">
	<?= tpl_text('Did you mean:') ?>
	<a href="?query=<?= $suggestedQuery ?>"><?= $suggestedText ?></a>
</div>
<? } ?>

<? if (isset($matchSet)) { ?>
	<div style="margin-top:8px;">
	<? if (count($matchSet)) { ?>
		<div class="Result">
			<?= tpl_text("Results %d-%d of %d matching <b>'%s'</b>", $offset+1, 
				$offset+count($matchSet), $count, $request['query']) ?><br />
			<? $this->display(tpl_design_path('gui/pager.tpl')) ?>
		</div>

		<div class="List">
			<? $this->iterate($matchSet,'list_view.tpl',array('highlight'=>$highlight)) ?>
		</div>

		<? $this->display(tpl_design_path('gui/pager.tpl')) ?>
	<? } else { ?>
		<em><?= tpl_text("No results matching <b>'%s'</b> were found", $request['query']) ?></em>
	<? } ?>
	</div>
<? } else { ?>
	<br />
	<div class="indent">
		- <a href="<?= tpl_link('course','search','question',array('filter'=>array('progress'=>1))) ?>">
			<?= tpl_text("Show 'Progress-Check' questions") ?></a><br />
		- <a href="<?= tpl_link('course','search','question',array('filter'=>array('diagnostic'=>1))) ?>">
			<?= tpl_text("Show 'Diagnostic-Test' questions") ?></a><br />
	</div>
<? } ?>