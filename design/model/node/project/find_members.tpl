<div class="Article">
	<? $this->render($node,'part_view_header.tpl') ?>
	<br />
	
	<h3><?= tpl_text('Add members to %s', $node->toString()) ?></h3>
	<div class="Block">
		<form method="post">
			<input type="text" name="query" value="<?= tpl_value($request['query']) ?>" />
			<input type="submit" value="<?= tpl_text('Search') ?>" />

			<? if (count($matchSet)) { ?>
				<div class="list indent" style="margin-top:10px;">
				<? foreach (array_keys($matchSet) as $key) { ?>
					<div class="Item">
						<input type="checkbox" name="users[]" value="<?= $matchSet[$key]->nodeId ?>" />
						<? $this->render($matchSet[$key],'head_view.tpl') ?>
						<? $this->render($matchSet[$key],'head_view_contact.tpl') ?><br />
					</div>
				<? } ?>
				</div>
				
				<input type="submit" name="post" value="<?= tpl_text('Proceed >>>') ?>" />
			<? } else if (isset($matchSet)) { ?>
				<em><?= tpl_text("No results were found containing <b>'%s'</b>", $request['query']) ?></em>
			<? } ?>
		</form>
		
		<? if (count($addedUsers)) { ?>
			<br />
			<h3><?= tpl_text('Added %d users', count($addedUsers)) ?></h3>
			<div class="indent">
				<? foreach (array_keys($addedUsers) as $key) { ?>
					<? $this->render($addedUsers[$key],'head_view.tpl') ?><br />
				<? } ?>
			</div>
		<? } ?>
	</div>
</div>