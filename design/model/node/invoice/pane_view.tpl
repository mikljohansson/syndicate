<table style="border:0;">
	<tr>
		<td style="padding:0; width:70%;">
			<div class="Title"><?= $this->quote($node->getTitle()) ?></div>
			<ul class="Actions">
				<? if ($node->isPermitted('write') && $node->isPaid()) { ?>
				<li><a href="<?= tpl_link_call($node->getHandler(),'invoke',$node->nodeId,'revoke') ?>"><?= tpl_text('Revoke invoice payment') ?></a></li>
				<? } ?>
				<li><a href="<?= $node->getInvoiceUri() ?>"><?= tpl_text('Download invoice (PDF)') ?></a></li>
				<? if ($node->isPermitted('print')) { ?>
				<li><a href="<?= tpl_link_call($node->getHandler(),'invoke',$node->nodeId,'printInvoice') ?>"><?= tpl_text('Print invoice') ?></a></li>
				<? } ?>
			</ul>

			<? if ($node->isPermitted('read') && count($node->getCategories())) { ?>
			<h4><?= $this->text('Categories') ?></h4>
			<? foreach (SyndLib::sort(iterator_to_array($node->getCategories())) as $category) { 
				if ($i++) print ', '; 
				?><a href="<?= tpl_link('issue','keyword',$category->nodeId) ?>" title="<?= tpl_attribute($category->getDescription()) ?>"><?= $category->toString() ?></a><?
			}} ?>

			<?= SyndLib::runHook('issue_view', $this, $node) ?>

			<? $content = $node->getContent(); if (null != $content->toString()) { ?>
				<h4>
					<?= $this->text('Description') ?>
					<? if ($node->isPermitted('write')) { ?>
					<a href="<?= tpl_link_call($node->getHandler(),'invoke',$node->nodeId,'forward') ?>"><img src="<?= tpl_design_uri('image/icon/16x16/forward.gif') 
						?>" width="16" height="16" alt="<?= $this->text('Forward') ?>" title="<?= $this->text('Forward this message') ?>" /></a>
					<a href="<?= tpl_link_call($node->getHandler(),'invoke',$node->nodeId,'reply') ?>"><img src="<?= tpl_design_uri('image/icon/16x16/reply.gif') 
						?>" width="16" height="16" alt="<?= $this->text('Reply') ?>" title="<?= $this->text('Reply to this message') ?>" /></a>
					<? $this->render($content,'icons.tpl',null,false) ?>
					<? } ?>
				</h4>
				<? $this->render($node->getContent(),'full_view.tpl',array('filter' => array($node, '_callback_filter'))) ?>
			<? } ?>

			<? 
			$duration = $node->getDuration();
			$estimate = $node->getEstimate();
			if ($duration || $estimate) { ?>
			<table style="width:1%; white-space:nowrap;">
				<tbody>
					<tr>
						<? if ($duration) { ?>
						<td>
							<h4><?= $this->text('Time spent') ?></h4>
							<?= tpl_duration($duration) ?>
						</td>
						<? } ?>
						<? if ($estimate) { ?>
						<td>
							<h4><?= $this->text('Time estimate') ?></h4>
							<?= tpl_duration($estimate) ?>
						</td>
						<? } ?>
					</tr>
				</tbody>
			</table>
			<? } ?>

			<? $this->render($node,'attachment/view.tpl') ?>
		</td>
		<td style="padding:0;">
			<h4><?= tpl_text('Items') ?></h4>
			<? $this->iterate($node->getLeasings(),'list_view.tpl') ?>
		</td>
	</tr>
</table>