<a href="<?= tpl_link('user','summary',$user->nodeId) ?>"><?= $user->toString() ?></a>
<ul class="Actions">
	<li><a href="<?= tpl_link('user','logout',array('stack'=>array('/'))) ?>"><?= tpl_text('Logout') ?></a></li>
</ul>