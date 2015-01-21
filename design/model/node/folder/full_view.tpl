<div class="Article">
	<div class="Header">
		<h1><?= $node->toString() ?></h1>
		<? if (null != $node->data['INFO_DESC']) { ?>
		<div class="Info"><?= tpl_text($node->data['INFO_DESC']) ?></div>
		<? } ?>
	</div>

	<?
	$this->append('tabs', array(
		'uri' => tpl_view($node->getHandler(),'view',$node->nodeId),
		'text' => tpl_text('Content'),
		'template' => array($node,'pane_view.tpl'),
		'selected' => null == $request[0]));

	$this->append('tabs', array(
		'uri' => tpl_view($node->getHandler(),'view',$node->nodeId,'config'),
		'text' => tpl_text('Configuration'),
		'template' => array($node,'pane_view_config.tpl'),
		'selected' => 'config' == $request[0]));

	if ($node->isPermitted('admin')) {
		$this->append('tabs', array(
			'uri' => tpl_view($node->getHandler(),'view',$node->nodeId,'admin'),
			'text' => tpl_text('Administer'),
			'template' => array($node,'pane_view_admin.tpl'),
			'selected' => 'admin' == $request[0]));
	}

	$this->display(tpl_design_path('gui/pane/tabbed.tpl'),$_data);
	?>
</div>

<? 
if (count($clsids = $node->getClasses())) {
	sort($clsids);
	
	foreach ($clsids as $clsid) {
		$storage = SyndNodeLib::getDefaultStorage($clsid);
		$database = $storage->getDatabase();

		$query = call_user_func(array(SyndNodeLib::loadClass($clsid), 'getEntityQuery'), $storage);
		$query->where('parent_node_id = '.$database->quote($node->nodeId));
		
		$cntquery = clone $query;
		$cntquery->column('COUNT(1)');
		$count = $database->getOne($cntquery->toString($database));

		if ($count) {
			$limit  = isset($_REQUEST["{$clsid}_limit"]) ? $_REQUEST["{$clsid}_limit"] : 100;
			$offset = isset($_REQUEST["{$clsid}_offset"]) ? $_REQUEST["{$clsid}_offset"] : 0;
			$query->column($query->getPrimaryKey());

			foreach ((array)tpl_sort_order($clsid) as $i => $column) {
				if (is_string($column) && !is_numeric($column) && !empty($column)) {
					$query->column($column);
					$query->order($column, !isset($this->_order[$i+1]) || !empty($this->_order[$i+1]));
				}
			}

			$children = $storage->getInstances($database->getCol($query->toString(),0,$offset,$limit));
			
			?>
			<div class="Result">
				<?= tpl_text("<b>".ucfirst($clsid)."s</b> (%d {$clsid}s in folder)", $count) ?>
				<? $this->display(tpl_design_path('gui/pager.tpl'),array(
					'limit'=>$limit,'offset'=>$offset,'count'=>$count,'offset_variable_name'=>"{$clsid}_offset")); ?>
			</div>
			<?

			print tpl_gui_table($clsid, $children, 'view.tpl', array('collection' => $node));
			print '<br />';
		} 
	} 
} 
?>