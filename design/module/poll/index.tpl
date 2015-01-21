<h2><?= tpl_text('My polls') ?></h2>
<dl class="Actions">
	<dt><a href="<?= tpl_link('poll','new') ?>"><?= tpl_text('Create new poll') ?></a></dt>
	<dd><?= tpl_text('Create and design a new poll with the questions you choose. You can choose from a multitude of question types and have statistics presented when people have replied to the poll.') ?>
</dl>

<? if (count($myPolls)) { ?>
	<hr />
	<? $this->iterate($myPolls,'item.tpl') ?>
<? } else { ?>
	<em><?= tpl_text('No polls found.') ?></em>
<? } ?>

