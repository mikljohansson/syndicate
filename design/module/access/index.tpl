<h1><?= tpl_text('Access control') ?></h1>
<?

$this->append('tabs', array(
	'uri' => tpl_link('system','access'),
	'text' => tpl_text('Modules'),
	'selected' => empty($request[0]) || 'modules' == $request[0],
	'template' => tpl_design_path('module/access/default_pane_modules.tpl')));

$this->append('tabs', array(
	'uri' => tpl_link('system','access','index','roles'),
	'text' => tpl_text('Roles'),
	'selected' => 'roles' == $request[0],
	'template' => tpl_design_path('module/access/default_pane_roles.tpl')));

$this->display(tpl_design_path('gui/pane/tabbed.tpl'));
