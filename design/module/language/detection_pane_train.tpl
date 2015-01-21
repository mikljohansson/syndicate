<p class="Help"><?= tpl_text('Train the language detection routine using text from a known language, for example cut-and-pasted news articles.') ?></p>

<? if (!empty($status)) { ?>
<p class="Result">
	<?= $status ?>
</p>
<? } ?>

<form method="post">
	<div class="RequiredField">
		<h3><?= tpl_text('Text to train') ?></h3>
		<textarea name="text" cols="76" rows="15"></textarea>
	</div>
	<div class="RequiredField">
		<h3><?= tpl_text('Language of text') ?></h3>
		<select name="locale">
			<? foreach ($locales as $key => $locale) { ?>
			<option value="<?= $key ?>"<?= $request['locale']==$key?' selected="selected"':'' ?>><?= $locale['name'] ?> (<?= $locale['code'] ?>)</option>
			<? } ?>
		</select>
	</div>
	<input type="submit" name="train" value="<?= tpl_text('Train') ?>" />
</form>