<div style="margin-bottom:1em;">
	<a href="<?= tpl_link($node->getHandler()) ?>"><?= tpl_text('Home') ?></a> &raquo;
	<a href="<?= tpl_link($node->getHandler(),'view',$node->nodeId) ?>"><?= $node->toString() ?></a>
</div>

<div class="indent">
	<?= $node->toString() ?>
</div>

<?
$questions = SyndLib::filter($node->getPage()->getQuestions(), 'isInheritedFrom', 'synd_node_open_ended_text');
$this->iterate($questions, 'item_replies.tpl');

?>