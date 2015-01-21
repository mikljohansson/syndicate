<div class="Item">
	<div class="Info"><?= tpl_chop($node->toString(),50) ?></div>
	<? if (!empty($options)) { ?>
	<table class="indent">
		<? foreach ($options as $id) { ?>
			<? if (null != ($option = $node->getOption($id))) { ?>
			<tr>
				<td style="padding-right:2em;"><?= $option['INFO_OPTION'] ?></td>
				<td><a href="<?= tpl_link_call($node->getHandler(),'invoke',
					$node->nodeId,'delStatisticsFilter',$id) 
					?>"><img src="<?= tpl_design_uri('image/icon/trash.gif') ?>" /></a></td>
			</tr>
			<? } ?>
		<? } ?>
	</table>
	<? } ?>
</div>