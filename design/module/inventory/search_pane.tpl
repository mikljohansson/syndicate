<?
switch ($section) {
	case 'all':
		$this->display(tpl_design_path('module/inventory/search_multiple_classes.tpl'), 
			array('classes' => array('item','lease','repair')));
		return;
	case 'lease':
		$this->display(tpl_design_path('module/inventory/search_multiple_classes.tpl'), 
			array('classes' => array('lease')));
		return;
	case 'issue':
		$this->display(tpl_design_path('module/inventory/search_multiple_classes.tpl'), 
			array('classes' => array('repair')));
		return;
}

$this->display(tpl_design_path('module/inventory/search_single_class.tpl'));
