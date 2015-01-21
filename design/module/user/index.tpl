<h1><?= $this->text('User directory') ?></h1>
<form action="<?= tpl_link('user') ?>" method="get">
	<input type="text" name="q" value="<?= tpl_attribute($request['q']) ?>" size="60" />
	<input type="submit" value="<?= tpl_text('Search') ?>" />
</form>

<? if (isset($collection)) { ?>
<div style="margin-top:1em;">
	<? if ($count) { 
		$result = $collection->getContents($offset, $limit); ?>
		<div class="Result">
			<? if ('' != trim($request['q'])) { ?>
			<?= tpl_text("Results %d-%d of %d matching <b>'%s'</b>", $offset+1, $offset+count($result), $count, tpl_chop($request['q'],60)) ?>
			<? } else { ?>
			<?= tpl_text("Results %d-%d of %d", $offset+1, $offset+count($result), $count) ?>
			<? } ?>
			<br />
			<? $this->display('gui/pager.tpl',array('count'=>$count)) ?>
		</div>
		<table>
			<thead>
				<tr class="<?= $this->cycle(array('even','odd')) ?>">
					<th><?= $this->text('Name') ?></th>
					<th><?= $this->text('Organization') ?></th>
					<th><?= $this->text('Phone') ?></th>
				</tr>
			</thead>
			<tbody>
				<? foreach ($result as $user) { ?>
				<tr class="<?= $this->cycle() ?>">
					<td><a href="<?= tpl_link('user','summary',$user->nodeId) ?>"><?= $user->toString() ?></a></td>
					<td><a href="<?= tpl_link('user','summary',$user->getOrganization()->nodeId) ?>"><?= $user->getOrganization()->toString() ?></a></td>
					<td><?= $user->getPhone() ?></td>
				</tr>
				<? } ?>
			</tbody>
		</table>
	<? } else { ?>
		<em><?= tpl_text("No results matching <b>'%s'</b> were found", $request['q']) ?></em>
	<? } ?>
</div>
<? } ?>
