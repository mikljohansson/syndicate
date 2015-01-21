<h1><?= tpl_text('Inventory administration') ?></h1>
<? 
$pane = clone $this;
$pane->append('tabs', array(
	'uri' => tpl_link('system','inventory','admin'),
	'text' => tpl_text('Receipts and Printing'),
	'selected' => null == $request[0],
	'template' => tpl_design_path('module/inventory/admin_pane_receipt.tpl')));

$pane->append('tabs', array(
	'uri' => tpl_link('system','inventory','admin','categories'),
	'text' => tpl_text('Categories'),
	'selected' => 'categories' == $request[0],
	'template' => tpl_design_path('module/inventory/admin_pane_categories.tpl')));

$pane->append('tabs', array(
	'uri' => tpl_link('system','inventory','admin','folders'),
	'text' => tpl_text('Folders'),
	'selected' => 'folders' == $request[0],
	'template' => tpl_design_path('module/inventory/admin_pane_folders.tpl')));

$pane->display(tpl_design_path('gui/pane/tabbed.tpl'));

