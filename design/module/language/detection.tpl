<h1><?= tpl_text('Language detection') ?></h1>
<?

$this->append('tabs', array(
	'uri' => tpl_link('system','language','detection'),
	'text' => tpl_text('Train'),
	'selected' => null == $request[0],
	'template' => tpl_design_path('module/language/detection_pane_train.tpl')));
$this->append('tabs', array(
	'uri' => tpl_link('system','language','detection','detect'),
	'text' => tpl_text('Detect'),
	'selected' => 'detect' == $request[0],
	'template' => tpl_design_path('module/language/detection_pane_detect.tpl')));

$this->display(tpl_design_path('gui/pane/tabbed.tpl'));
