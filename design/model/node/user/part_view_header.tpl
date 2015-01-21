<div class="page_text_header" style="margin-top:4px; text-align:left;">
	<? if ('homepage' == $selected) print '<span class="selected">'; ?>
	<a href="<?= tpl_link('user','view',$node->nodeId) ?>"><?= tpl_text('Homepage') ?></a>
	<? if ('homepage' == $selected) print '</span>'; ?>
	<b>:</b>
	<? if ('blog' == $selected) print '<span class="selected">'; ?>
	<a href="<?= tpl_link('user','view',$node->nodeId,'blog') ?>"><?= tpl_text('Blog') ?></a>
	<? if ('blog' == $selected) print '</span>'; ?>
	<b>:</b>
	<? if ('media' == $selected) print '<span class="selected">'; ?>
	<a href="<?= tpl_link('user','view',$node->nodeId,'media') ?>"><?= tpl_text('Audio/Visual') ?></a>
	<? if ('media' == $selected) print '</span>'; ?>
	<b>:</b>
	<? if ('texts' == $selected) print '<span class="selected">'; ?>
	<a href="<?= tpl_link('user','view',$node->nodeId,'texts') ?>"><?= tpl_text('Texts') ?></a>
	<? if ('texts' == $selected) print '</span>'; ?>
	<b>:</b>
	<? if ('comments' == $selected) print '<span class="selected">'; ?>
	<a href="<?= tpl_link('user','view',$node->nodeId,'comments') ?>"><?= tpl_text('Comments') ?></a>
	<? if ('comments' == $selected) print '</span>'; ?>
</div>