<h3><?= tpl_text('Project members') ?></h3>
<? if (count($users = iterator_to_array($node->getMembers()))) { ?>
<table>
	<tbody>
		<? foreach (SyndLib::sort($users) as $user) { ?>
		<tr>
			<td style="padding-right:2em;"><? $this->render($user,'contact.tpl') ?></td>
			<? if ($node->isLocalMember($user)) { ?>
			<td><a href="<?= tpl_link_call($node->getHandler(),'invoke',$node->nodeId,'removeMember',array('user'=>$user->nodeId)) 
				?>"><img src="<?= tpl_design_uri('image/icon/trash.gif') ?>" alt="<?= tpl_text('Delete') ?>" /></a></td>
			<? } else { ?>
			<td>&nbsp;</td>
			<? } ?>
		</tr>
		<? } ?>
	</tbody>
</table>
<? } ?>

<form action="<?= tpl_link('issue','project',$node->getProjectId(),'admin') ?>" method="post">
	<h3><?= tpl_text('New member') ?></h3>
	<div class="indent">
		<input type="text" name="query" value="<?= tpl_value($request['query']) ?>" size="40" />
		<input type="submit" value="<?= tpl_text('Search') ?>" />
	</div>
</form>

<? if (isset($request['query'])) { ?>
	<? 
	$collection = $node->findMemberInstances($request['query']);
	if (count($users = $collection->getContents(0,20))) { ?>
		<form action="<?= tpl_view_call($node->getHandler(),'invoke',$node->nodeId,'addMembers') ?>" method="post">
			<div class="list indent" style="margin-top:10px;">
				<? foreach (array_keys($users) as $key) { ?>
				<div class="Item">
					<input type="checkbox" name="users[]" value="<?= $users[$key]->nodeId ?>" />
					<? $this->render($users[$key],'contact.tpl') ?><br />
				</div>
				<? } ?>
			</div>
			<input type="submit" name="post" value="<?= tpl_text('Add >>>') ?>" />
		</form>
	<? } else if (isset($matchSet)) { ?>
		<em><?= tpl_text("No results were found containing <b>'%s'</b>", $request['query']) ?></em>
	<? } ?>
	<br />
<? } ?>
