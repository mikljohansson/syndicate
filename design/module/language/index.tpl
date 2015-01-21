<? 
function _synd_language_input($name, $tabindex, $string, $translation) {
	$length = max(strlen($string), strlen($translation));
	if ($length > 50)
		return tpl_form_textarea($name, $translation, array('cols' => 22, 'rows' => ceil($length/30), 'tabindex' => $tabindex));
	return '<input type="text" name="'.$name.'" value="'.tpl_attribute($translation).'" size="30" tabindex="'.$tabindex.'" />';
}

?>
<h3><?= tpl_text('Text filter') ?></h3>
<div class="indent" style="margin-bottom:1em;">
	<form action="<?= tpl_link('system','language') ?>" method="get">
		<input type="text" name="filter" value="<?= $request['filter'] ?>" />
		<input type="submit" value="<?= tpl_text('Update') ?>" />
	</form>
</div>

<form action="<?= tpl_view_call('language','saveStrings') ?>" method="post">
	<div class="Result">
		<?= tpl_text('Displaying %d-%d of %d strings available for localization', 
			$offset+1, $offset+min($count-$offset,$limit), $count) ?>
		<? $this->display(tpl_design_path('gui/pager.tpl')) ?>
	</div>

	<table>
	<thead>
		<tr>
			<th><?= tpl_text('String') ?></th>
			<? foreach ($locales as $info) { ?>
			<th><?= $info['name'] ?></th>
			<? } ?>
			<th>&nbsp;</th>
			<th style="padding-left:1em;"><?= tpl_text('Location') ?></th>
		</tr>
	</thead>
	<tbody>
		<? foreach ($strings as $string) { ?>
		<tr class="<?= tpl_cycle(array('odd','even')) ?>">
			<td width="250"><?= htmlspecialchars($string['STRING']) ?></td>
			<? $i=1; foreach (array_keys($locales) as $code) { ?>
			<td><?= _synd_language_input("strings[$code][{$string['LID']}]", $i++, 
					$string['STRING'], $translations[$code][$string['STRING']]) ?></td>
			<? } ?>
			<td><a href="<?= tpl_link_call('language','deleteString',array('lid'=>$string['LID'])) 
				?>"><img src="<?= tpl_design_uri('image/icon/trash.gif') ?>" alt="<?= tpl_text('Delete') ?>" /></a></td>
			<td style="padding-left:1em;">
				<span title="<?= tpl_value($string['LOCATION']) ?>"><?= tpl_chop($string['LOCATION'],30) ?></span>
			</td>
		</tr>
		<? } ?>
	</tbody>
	</table>

	<br />
	<span title="<?= tpl_text('Accesskey: %s', 's') ?>">
		<input type="submit" accesskey="s" name="post" value="<?= tpl_text('Save') ?>" />
	</span>
</form>