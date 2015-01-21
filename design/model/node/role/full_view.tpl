<div class="Article">
	<div class="Header"><h3><?= $node->toString() ?></h3></div>
	<div class="Abstract"><?= $node->getDescription() ?></div>
	<? if ($node->isPermitted('write')) { ?>
	<ul class="Actions">
		<li><a href="<?= tpl_link_call($node->getHandler(),'edit',$node->nodeId) ?>"><?= tpl_text('Edit this role') ?></a></li>
	</ul>
	<? } ?>
	
	<h3><?= tpl_text('Members') ?></h3>
	<table>
	<? foreach (array_keys($members = $node->getMembers()) as $key) { ?>
		<tr>
			<td style="padding-right:2em;"><? $this->render($members[$key],'contact.tpl') ?></td>
			<td><a href="<?= tpl_link_call($node->getHandler(),'invoke',$node->nodeId,'remove',$members[$key]->nodeId) 
					?>"><img src="<?= tpl_design_uri('image/icon/trash.gif') ?>" /></a></td>
		</tr>
	<? } ?>
	</table>

	<? if ($node->isPermitted('write')) { ?>
	<form method="post">
		<h4><?= tpl_text('Assign user to role') ?></h4>
		<div class="indent">
			<input type="text" name="query" value="<?= tpl_attribute($_REQUEST['query']) ?>" />
			<input type="submit" value="<?= tpl_text('Search') ?>" />

			<div style="margin-top:0.5em; margin-bottom:1em;">
			<? if (!empty($_REQUEST['query']) && empty($_REQUEST['post'])) { ?>
				<? 
				$path = "node/invoke/{$node->nodeId}/add/";
				$collection = $node->findRoles($_REQUEST['query']); 
				if (count($roles = $collection->getContents(0,20))) { ?>
					<? foreach (array_keys($roles) as $key) { ?>
					<?= tpl_form_checkbox("mplex[$path][roles][]",
						isset($_REQUEST['mplex'][$path]['roles']) && 
						in_array($roles[$key]->nodeId, $_REQUEST['mplex'][$path]['roles']),
						$roles[$key]->nodeId) ?>
						<? $this->render($roles[$key],'contact.tpl') ?><br />
					<? } ?>

					<br />
					<input type="submit" name="post" value="<?= tpl_text('Save') ?>" />
				<? } else { ?>
					<em><?= tpl_text("No results matching <b>'%s'</b> were found", 
						$_REQUEST['query']) ?></em>
				<? } ?>
			<? } ?>
			</div>
		</div>
	</form>
	<? } ?>
</div>