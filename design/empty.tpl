<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1//EN" "http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="<?= tpl_def(SyndLib::runHook('getlocale'),'en') ?>">
<head>
	<title><?= tpl_default(SyndLib::runHook('html_head_title'),$title) ?></title>
	<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
	<meta name="description" content="<?= tpl_attribute(SyndLib::runHook('html_meta_description')) ?>" />
	<meta name="keywords" content="<?= tpl_attribute(SyndLib::runHook('html_meta_keywords')) ?>" />
	<link rel="stylesheet" href="<?= tpl_design_uri('layout/interface.css') ?>" type="text/css" media="all" />
	<link rel="stylesheet" href="<?= tpl_design_uri('layout/screen.css') ?>" type="text/css" media="screen, projection, tv" />
	<link rel="stylesheet" href="<?= tpl_design_uri('layout/print.css') ?>" type="text/css" media="print" />
<?= SyndLib::runHook('html_head', $this) ?>
</head>
<body>
	<?= $content ?>
<?= SyndLib::runHook('html_body_post') ?>
</body>
</html>