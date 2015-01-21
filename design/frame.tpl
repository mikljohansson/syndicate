<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Frameset//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-frameset.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="<?= tpl_def(SyndLib::runHook('getlocale'),'en') ?>">
<head>
	<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
	<title><?= tpl_default(SyndLib::runHook('html_head_title'),$title) ?></title>
<?= SyndLib::runHook('html_head', $this) ?>
</head>
<?= $content ?>
</html>