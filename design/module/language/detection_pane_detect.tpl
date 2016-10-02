<? if (isset($scores)) { ?>
	<? if (empty($scores)) { ?>
	<div class="Notice">
		<?= tpl_text('No locale could be detected, please train the detection using a known dataset') ?>
	</div>
	<? } else { ?>
	<div class="Result">
		<h3><?= tpl_text("The detected locale was '%s'", SyndLib::end(array_keys($scores))) ?></h3>
		<table>
			<tr>
				<th><?= tpl_text('Name') ?></th>
				<th><?= tpl_text('Code') ?></th>
				<th><?= tpl_text('Score') ?></th>
			</tr>
			<? foreach ($locales as $locale => $info) { ?>
			<tr>
				<td><?= $info['name'] ?></td>
				<td><?= $info['code'] ?></td>
				<td><?= $scores[$locale] ?></td>
			</tr>
			<? } ?>
		</table>
	</div>
	<? } ?>
<? } ?>

<form method="post">
	<div class="RequiredField">
		<h3><?= tpl_text('Text to test') ?></h3>
		<textarea name="text" cols="76" rows="15"><?= synd_htmlspecialchars($request['text']) ?></textarea>
	</div>
	<input type="submit" name="detect" value="<?= tpl_text('Detect') ?>" />
</form>