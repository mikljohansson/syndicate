<h1><?= tpl_text('Search engine control center') ?></h1>
<?

$this->append('tabs', array(
	'uri' => tpl_link('system','search','admin'),
	'text' => tpl_text('Index options'),
	'selected' => null == $request[0],
	'template' => tpl_design_path('module/search/admin_pane_index.tpl')));
$this->append('tabs', array(
	'uri' => tpl_link('system','search','admin','web'),
	'text' => tpl_text('Web and Spider options'),
	'selected' => 'web' == $request[0],
	'template' => tpl_design_path('module/search/admin_pane_web.tpl')));
$this->display(tpl_design_path('gui/pane/tabbed.tpl'), $_data);
