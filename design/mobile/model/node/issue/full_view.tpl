<? global $synd_user; ?>
<h3><?= $node->data['INFO_HEAD'] ?><h3>

<? if (!$node->isClosed() && $node->isPermitted('write')) { ?>
<ul class="Actions">
	<li><a href="<?= tpl_link_jump($node->getHandler(),'invoke',$node->nodeId,'cancel') ?>">
		<?= tpl_text('Cancel issue') ?></a></li>
</ul>
<? } ?>

<? $customer = $node->getCustomer(); ?>
<? if ($customer->nodeId != $synd_user->nodeId) { ?>
<b>C:</b>
	<? $this->render($customer,'contact.tpl') ?>
<? } ?>

<? $assigned = $node->getAssigned(); ?>
<? if (!$assigned->isNull()) { ?>
<b>A:</b>
	<? $this->render($assigned,'contact.tpl') ?>
<? } ?>

<? if (!$node->isClosed()) { ?>
<b><?= tpl_text('Due') ?></b> 
	<?= date('Y-m-d', $node->data['TS_RESOLVE_BY']) ?><br />
<? } ?>

<? $content = $node->getContent(); if (null != trim($content->toString())) { ?>
<b><?= tpl_text('Description') ?></b><br />
	<? $this->render($content,'full_view.tpl') ?>
<br />
<? } ?>

<? if (count($node->getNotes())) { ?>
<b><?= tpl_text('Comments') ?></b><br />
	<? $this->iterate($node->getNotes(),'full_view.tpl') ?>
<? } ?>
