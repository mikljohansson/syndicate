<? $project = $node->getParent(); ?>
<div class="Notice">
	<h2><?= tpl_text('Thank you for your feedback') ?></h2>
	<? if (null != $project->getEmail()) { ?>
	<p><?= tpl_text('You, or someone using the same link you clicked on, have already left feedback for this issue. Please contact our customer service (<a href="%s">%s</a>) if you have futher feedback to give.', 'mailto:'.$project->getEmail(), $project->getEmail()) ?></p>
	<? } else { ?>
	<p><?= tpl_text('You, or someone using the same link you clicked on, have already left feedback for this issue. Please contact our customer service if you have futher feedback to give.') ?></p>
	<? } ?>
</div>