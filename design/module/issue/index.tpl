<? include tpl_gui_path('synd_node_issue','confirm_email_status.tpl') ?>
<h1><?= tpl_text('Project and issue tracker') ?></h1>
<? 
$this->append('tabs', array(
	'uri' => tpl_link('issue'),
	'text' => tpl_text('Projects'),
	'selected' => null == $pane,
	'template' => tpl_design_path('module/issue/default_pane_projects.tpl')));
$this->append('tabs', array(
	'uri' => tpl_link('issue','issues'),
	'text' => tpl_text('My issues'),
	'partial' => array(tpl_uri_merge(null, 'issue/issues/'), 'assigned'),
	'selected' => 'issues' == $pane,
	'template' => tpl_design_path('module/issue/default_pane_issues.tpl')));
$this->append('tabs', array(
	'uri' => tpl_link('issue','unassigned'),
	'text' => tpl_text('Unassigned'),
	'partial' => array(tpl_uri_merge(null, 'issue/unassigned/'), 'unassigned'),
	'selected' => 'unassigned' == $pane,
	'template' => tpl_design_path('module/issue/default_pane_unassigned.tpl')));
$this->append('tabs', array(
	'uri' => tpl_link('issue','search'),
	'text' => tpl_text('Search'),
	'selected' => 'search' == $pane,
	'template' => tpl_design_path('module/issue/default_pane_search.tpl')));

$this->display(tpl_design_path('gui/pane/tabbed.tpl'));
