<div class="Success">
	<h2><?= tpl_text('Your issue has been reopened') ?></h2>
	<p><?= tpl_text('The issue has been reopened and you might be contacted by our personel to guarantee a satisfactory solution.') ?></p>
	<? if ($note) { ?>
	<h4><?= tpl_text('Feedback or message') ?></h4>
	<blockquote><?= $note ?></blockquote>
	<? } ?>
</div>