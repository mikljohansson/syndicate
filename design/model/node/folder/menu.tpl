<? 
if (!count($children = SyndLib::sort(SyndLib::filter($node->getChildren(),'isPermitted','read'),'toString')))
	$class = 'Leaf';
else if (isset($expand[$node->nodeId]))
	$class = 'Expanded';
else
	$class = 'Collapsed';
?>
<li class="<?= $class ?>"><a href="<?= tpl_link($node->getHandler(),'view',$node->nodeId) 
	?>" title="<?= tpl_value($node->data['INFO_DESC']) ?>"><?= tpl_def(tpl_chop($node->toString(),18)) ?></a></li>
	<? if (isset($expand[$node->nodeId]) && !empty($children)) { ?>
	<ul>
		<? $this->iterate($children,'menu.tpl',$_data) ?>
	</ul>
	<? } ?>