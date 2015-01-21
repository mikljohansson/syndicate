<div class="Title"><?= $this->quote($node->getTitle()) ?></div>
<a href="<?= tpl_link($node->getHandler(),'invoke',$node->nodeId,'newIssue') ?>" accesskey="d" style="display:none;"></a>
<a href="<?= tpl_link($node->getHandler(),'invoke',$node->nodeId,'clone') ?>" accesskey="c" style="display:none;"></a>

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
