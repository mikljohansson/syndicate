<h1><?= tpl_text('Search the inventory') ?></h1>
<? 
$this->append('tabs', array(
	'uri' => tpl_link('inventory','search'),
	'text' => tpl_text('Search'),
	'selected' => null == $request[0],
	'template' => tpl_design_path('module/inventory/search_pane.tpl')));

$this->append('tabs', array(
	'uri' => tpl_link('inventory','search','specific'),
	'text' => tpl_text('Specify list'),
	'selected' => 'specific' == $request[0],
	'template' => tpl_design_path('module/inventory/search_pane_specific.tpl')));

$this->display(tpl_design_path('gui/pane/tabbed.tpl'));
