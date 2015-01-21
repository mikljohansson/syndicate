<? require_once 'design/gui/MenuBuilder.class.inc'; ?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1//EN" "http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="<?= tpl_def(SyndLib::runHook('getlocale'),'en') ?>">
<head>
	<title><?= $this->quote(tpl_default($title,SyndLib::runHook('html_head_title'))) ?></title>
	<meta http-equiv="Content-Type" content="text/html; charset=<?= tpl_default(ini_get('default_charset'),'utf-8') ?>" />
<? if (null != ($description = SyndLib::runHook('html_meta_description'))) { ?>
	<meta name="description" content="<?= tpl_attribute($description) ?>" />
<? } ?>
<? if (null != ($keywords = SyndLib::runHook('html_meta_description'))) { ?>
	<meta name="keywords" content="<?= tpl_attribute($keywords) ?>" />
<? } ?>
	<link rel="stylesheet" href="<?= tpl_design_uri('layout/interface.css') ?>" type="text/css" media="all" />
	<link rel="stylesheet" href="<?= tpl_design_uri('layout/screen.css') ?>" type="text/css" media="screen, projection, tv" />
	<link rel="stylesheet" href="<?= tpl_design_uri('layout/print.css') ?>" type="text/css" media="print" />
<?= SyndLib::runHook('html_head', $this) ?>
</head>
<body>
	<div id="Container">
		<div id="Header">
			<div id="Search"><?= SyndLib::runHook('header', $this) ?></div>
			<div id="Navigation">
				<h1><a href="/" title="<?= php_uname('n') ?>"><?= $_SERVER['SERVER_NAME'] ?></a></h1>
				<? SyndLib::runHook('menu_header', $this, $menu = new ListMenu($path)); $menu->display(); ?>
			</div>
		</div>
		<table id="Content">
		<tr>
			<td>
				<? $this->display(tpl_design_path('gui/breadcrumbs.tpl')) ?>
				<?= $content ?>
			</td>
			<td id="Sidebar">
				<? SyndLib::runHook('menu', $menu = new ListMenu($path)); if ($menu->hasChildren()) { ?>
				<div class="Block">
					<div class="Contents">
						<? $menu->display(); ?>
					</div>
				</div>
				<? } ?>
				<? foreach ((array)SyndLib::runHook('block', $this) as $block) { ?>
				<div class="Block">
					<? if (isset($block['text'])) { ?>
					<h3><?= $block['text'] ?></h3>
					<? } ?>
					<div class="Contents">
						<?= $block['content'] ?>
					</div>
				</div>
				<? } ?>
			</td>
		</tr>
		</table>
	</div>
<?= SyndLib::runHook('html_body_post') ?>
</body>
</html>
