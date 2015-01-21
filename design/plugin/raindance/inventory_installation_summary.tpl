<? if ($invalid) { ?>
	<div class="Notice">
		<h4><?= tpl_text('The installation has not been registered in Raindance') ?></h4>
	</div>
<? } else { ?>
	<div class="Result">
		<div class="Info"><?= tpl_text('Information from raindance') ?></div>
		<h3><?= $title ?></h3>
		<p>
			<? if ($validfrom) { ?><?= tpl_text("Valid from '%s'", tpl_strftime('%Y-%m-%d', $validfrom)) ?><? } ?>
			<? if ($validto) { ?><?= tpl_text("to '%s'.", tpl_strftime('%Y-%m-%d', $validto)) ?><? } ?>
		</p>
	</div>
<? } ?>