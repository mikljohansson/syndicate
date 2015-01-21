<!DOCTYPE html PUBLIC "-//WAPFORUM//DTD XHTML Mobile 1.0//EN" "http://www.wapforum.org/DTD/xhtml-mobile10.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="<?= tpl_def(SyndLib::runHook('getlocale'),'en') ?>">
<head>
	<title><?= tpl_default(SyndLib::runHook('html_head_title'),$title) ?></title>
	<meta http-equiv="Content-type" content="text/html; charset=iso-8859-1" />
	<link rel="stylesheet" href="<?= tpl_design_uri('layout/screen.css') ?>" type="text/css" />
<?= SyndLib::runHook('html_head', $this) ?>
</head>
<body>
	<? $this->display(tpl_design_path('gui/breadcrumbs.tpl')) ?>
	<?= $content ?>
<?= SyndLib::runHook('html_body_post') ?>
</body>
</html>
