<h1><?= tpl_text('Storage administration') ?></h1>
<?

$this->append('tabs', array(
	'uri' => tpl_link('system','search','admin'),
	'text' => tpl_text('Storage options'),
	'selected' => null == $request[0],
	'template' => tpl_design_path('module/tracker/admin_pane_options.tpl')));

$this->display(tpl_design_path('gui/pane/tabbed.tpl'), $_data);
