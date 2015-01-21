<tr>
	<td width="60"><?= $node->getPageNumber() ?></td>
	<td>
		<a href="#<?= $node->getPageNumber() ?>">
		<?= $node->data['INFO_HEAD'] ?>
		</a>
	</td>
</tr>
<? if ($levels >= 1) { ?>
	<? $this->iterate($node->getChildren(),'part_view_index.tpl',array('levels'=>$levels-1)) ?>
<? } ?>