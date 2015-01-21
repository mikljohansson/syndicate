<? global $synd_user; ?>
<? if ($node->isPermitted('write')) { ?>
<ul class="Actions">
	<li><a href="<?= tpl_link('issue','invoke',$node->nodeId,'newIssue') ?>" title="<?= 
		$this->text('Accesskey: %s','D') ?>" accesskey="d"><?= $this->text('Create subissue of this issue') ?></a></li>
	<li><a href="<?= tpl_link('issue','invoke',$node->nodeId,'clone') ?>" title="<?= 
		$this->text('Accesskey: %s','C') ?>" accesskey="c"><?= $this->text('Create issue using this as template') ?></a></li>
	<? if ($node->isPermitted('admin')) { ?>
	<li><a href="<?= tpl_link('issue','delete',$node->nodeId) ?>"><?= $this->text('Delete issue permanently') ?></a></li>
	<? } ?>
	<li><a href="<?= tpl_link('search',array('rset'=>array($node->id()))) ?>"><?= $this->text('Search for similar issues') ?></a></li>
</ul>
<? } ?>

<h4><?= $this->text('External link for non-users') ?></h4>
<a href="<?= tpl_view('issue',$node->objectId(),$node->getAuthenticationToken()) ?>"><?= tpl_request_host() ?><?= tpl_view('issue',$node->objectId(),$node->getAuthenticationToken()) ?></a>

<? if (null != ($email = $node->getDirectAddress())) { ?>
<h4><?= $this->text('Direct e-mail address') ?></h4>
<?= tpl_email($email) ?>
<? } ?>

<h4><?= $this->text('E-mail notifications') ?></h4>
<? 
$mailNotifier = $node->getMailNotifier();
if (count($events = $mailNotifier->getRegisteredEvents())) {
	foreach ($mailNotifier->getRegisteredEvents() as $event) { ?>
	<div style="margin-top:0.4em;">
		<?= $this->text('When the issue is %s to:', strtolower($this->text($event))) ?>
	</div>
	<div style="margin-left:1em;">
		<? foreach (array_keys($listeners = $mailNotifier->getListeners($event)) as $key) { ?>
			<?= $listeners[$key]->toString() ?>	
			<? if (!$synd_user->isNull()) { ?>
				&lt;<?= tpl_email($listeners[$key]->getEmail()) ?>&gt;
			<? } ?>
			<? 
			$project = $node->getParent();
			if ($project->isPermitted('write')) { ?>
			&nbsp;&nbsp; <a href="<?= tpl_link_call('issue','invoke',$node->nodeId,'delListener',$event,$listeners[$key]->id()) 
				?>"><img src="<?= tpl_design_uri('image/icon/trash.gif') ?>" alt="<?= $this->text('Remove') ?>" /></a>
			<? } ?><br />
		<? } ?>
	</div>
	<? } ?>
<? } else { ?>
<p><em><?= $this->text('No e-mails are currently sent automatically') ?></em></p>
<? } ?>
