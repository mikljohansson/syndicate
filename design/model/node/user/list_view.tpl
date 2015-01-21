<div class="Item">
	<? if (null != $node->data['INFO_PHOTO']) { ?>
	<table>
	<tr>
		<td>
			<a href="<?= tpl_link('user','view',$node->nodeId) ?>">
			<img src="<?= $node->data['INFO_PHOTO']->getResizedUri(60) ?>" width="60" alt="<?= $node->toString() ?>" />
			</a>
		</td>
		<td>
	<? } ?>
			<div class="Header">
				<a href="<?= tpl_link('user','view',$node->nodeId) ?>"><?= $node->toString() ?></a>
			</div>
			<div class="desc">
				<?= nl2br($node->data['INFO_DESC']) ?>
			</div>
	<? if (null != $node->data['INFO_PHOTO']) { ?>
		</td>
	</tr>
	</table>
	<? } ?>
</div>