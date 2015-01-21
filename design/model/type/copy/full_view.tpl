<div class="nowrap" style="margin-bottom:5px;">
	<b><?= tpl_text('Copied documents') ?></b>
</div>
<? if (count($list = $node->getInstances())) { ?>
<div class="indent" style="margin-bottom:5px;">
	<? 
	foreach (array_keys($list) as $key) { 
		if ($i++ >= 10) break; ?>
		<? $this->render($list[$key],'head_view.tpl',array('maxLength'=>18)) ?><br />
	<? } ?>
	<? if ($i > count($list)) { ?>
		<div class=""><?= tpl_text('%d more items',count($list)-$i+1) ?> ...</div>
	<? } ?>
</div>
<? } ?>

<ul class="Actions">
	<? if ($node->isValidTarget($target)) { ?>
	<li title="<?= tpl_text('Paste into the current context') ?>"><a href="javascript:synd_ole_call('<?= 
		tpl_view_call('system','invoke',$node->id(),'apply',$target->id(),(int)$force) ?>');"><?= tpl_text('Paste') ?></a></li>
	<? } else { ?>
	<li title="<?= tpl_text('The content can not be pasted into the current context') ?>"><?= tpl_text('Paste') ?></li>
	<? } ?>
	<li><a href="<?= tpl_link_call('system','invoke',$node->id(),'cancel') ?>"><?= tpl_text('Cancel') ?></a></li>
</ul>
