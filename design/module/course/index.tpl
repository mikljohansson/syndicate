<? 
global $synd_user;
$module = Module::getInstance('course');
?>
<h1><?= tpl_text('My courses') ?></h1>
<div style="padding-left:2em; width:600px;">
	<? foreach (array_keys(SyndLib::sort($myCourses)) as $key) { ?>
	<div class="Item">
		<div class="Header">
			<h3><a href="<?= tpl_link($myCourses[$key]->getHandler(),'view',$myCourses[$key]->getCourseId()) ?>"><?= $myCourses[$key]->getTitle() ?></a></h3>
		</div>
		<? if (null != $myCourses[$key]->getDescription()) { ?>
		<div class="Abstract">
			<?= tpl_html_format($myCourses[$key]->getDescription()) ?>
		</div>
		<? } ?>
		<ul class="Actions">
			<li><a href="<?= tpl_link_call($myCourses[$key]->getHandler(),'invoke',$myCourses[$key]->nodeId,'unregister') ?>"><?= 
				tpl_text('Unregister from %s', $myCourses[$key]->toString()) ?></a></li>
		</ul>
	</div>	
	<? } ?>
	<? if (empty($myCourses)) { ?>
	<div class="Info"><?= tpl_text('You have not subscribed to any courses yet.') ?></div>
	<? } ?>
</div>

<h1><?= tpl_text('Other courses') ?></h1>
<div style="padding-left:2em; width:600px;">
	<? foreach (array_keys(SyndLib::sort($courses)) as $key) { ?>
	<div class="Item">
		<div class="Header">
			<h3><a href="<?= tpl_link($courses[$key]->getHandler(),'view',$courses[$key]->getCourseId()) ?>"><?= $courses[$key]->getTitle() ?></a></h3>
		</div>
		<? if (null != $courses[$key]->getDescription()) { ?>
		<div class="Abstract">
			<?= tpl_html_format($courses[$key]->getDescription()) ?>
		</div>
		<? } ?>
		<ul class="Actions">
			<li><a href="<?= tpl_link($courses[$key]->getHandler(),'invoke',$courses[$key]->nodeId,'register') ?>"><?= 
				tpl_text('Register to %s', $courses[$key]->toString()) ?></a></li>
		</ul>
	</div>	
	<? } ?>
	<? if (empty($courses)) { ?>
	<div class="Info"><?= tpl_text('No courses found') ?></div>
	<? } ?>
</div>

<? if ($synd_user->isNull()) { ?>
<div style="width:600px; margin-top:10px;">
	<h3><?= tpl_text('Note:') ?></h3>
	<div class="indent">
		<?= tpl_text('You are not logged in, you might have to do so in order to gain access to most courses. Please login using your regular account if you have one, otherwise you should have gotten an email containing an account and login instructions.') ?>
	</div>
</div>
<? } ?>