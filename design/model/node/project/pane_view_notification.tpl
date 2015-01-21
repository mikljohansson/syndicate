<? 
global $synd_user; 
$module = Module::getInstance('issue');
$mailNotifier = $node->getMailNotifier();

?>
<form action="<?= tpl_view_call('issue','invoke',$node->nodeId,'saveNotification',$synd_user->id()) ?>" method="post">
	<p class="Help"><?= tpl_text("Notification is sent via email to '%s'", $synd_user->getEmail()) ?></p>
	<? foreach ($module->getDefinedEvents($node) as $event => $description) { ?>
	<?= tpl_form_checkbox("events[]",$mailNotifier->isRegisteredPermanent($event,$synd_user),$event,"events[$event]") ?> 
		<label for="events[<?= $event ?>]"><?= $description ?></label><br />
	<? } ?>
	<p><input type="submit" value="<?= tpl_text('Save') ?>" /></p>
</form>

<? if ($node->isPermitted('admin')) { 
	tpl_load_script(tpl_design_uri('js/json.js')); 
	tpl_load_script(tpl_design_uri('js/autocomplete.js')); ?>
	<h3><?= tpl_text('Registered e-mail addresses') ?></h3>
	<table class="Enumeration">
	<? foreach ($module->getDefinedEvents($node) as $event => $description) { ?>
		<? if (count($listeners = $mailNotifier->getListeners($event))) { ?>
		<tr>
			<td colspan="2"><h4><?= tpl_text($description) ?></h4></td>
		</tr>
		<? foreach ($listeners as $listener) { $i++; ?>
		<tr>
			<td><?= $listener->toString() ?> &lt;<?= tpl_email($listener->getEmail()) ?>&gt;</td>
			<td><a href="<?= tpl_link_call('issue','invoke',$node->nodeId,'removeNotification',$event,$listener->id()) 
				?>"><img src="<?= tpl_design_uri('image/icon/trash.gif') ?>" /></a></td>
		</tr>
		<? } ?>
		<? } ?>
	<? } ?>
		<? if (!$i) { ?>
		<tr>
			<td><em><?= tpl_text('No e-mails are currently sent automatically') ?></em></td>
		</tr>
		<? } ?>
	</table>

	<form action="<?= tpl_view_call('issue','invoke',$node->nodeId,'saveNotification') ?>" method="post">
		<h3><?= tpl_text('Register additional e-mail') ?></h3>
		<select name="event">
			<? foreach ($module->getDefinedEvents($node) as $event => $description) { ?>
			<option value="<?= $event ?>"><?= $description ?></option>
			<? } ?>	
		</select>
		<input type="text" name="email" id="email" size="40" />
		<input type="submit" value="<?= tpl_text('Add') ?>" />
	</form>
	<script type="text/javascript">
	<!--
		if (document.getElementById) {
			window.onload = function() {
				new AutoComplete(document.getElementById('email'), '<?= tpl_view('rpc','json',$node->id()) ?>', 'findSuggestedUsers');
			};
		}
	//-->
	</script>
<? } ?>
