<? tpl_load_script(tpl_design_uri('js/form.js')) ?>

<? if ($node->isPermitted('admin')) { ?>
<h3><?= tpl_text('Administrative actions') ?></h3>
<ul class="Actions">
	<li><a href="<?= tpl_link_call($node->getHandler(),'delete',$node->nodeId,
		tpl_view($node->getHandler())) ?>"><?= tpl_text('Delete this poll') ?></a></li>
	<li><a href="<?= tpl_link_call($node->getHandler(),'invoke',$node->nodeId,'clearAttempts') ?>">
		<?= tpl_text('Delete all replies to this poll') ?></a></li>
</ul>
<? } ?>

<form action="<?= tpl_link_call($node->getHandler(),'edit',$node->nodeId) ?>" method="post">
	<div style="margin-top:10px; margin-bottom:10px;">
		<h3><?= tpl_text('Publication settings') ?></h3>
		<div class="Notice">
			<? $uri = tpl_request_host().tpl_link($node->getHandler(),$node->objectId()); ?>
			<?= tpl_translate('The address for replying to this poll is <a href="%s">%s</a>, if a slimmed down view is desirable use <a href="%s?print=1">%s?print=1</a>', $uri, $uri, $uri, $uri) ?>
		</div>
		<div class="indent">
			<?= tpl_form_checkbox('data[FLAG_ANONYMOUS]',$node->data['FLAG_ANONYMOUS']) ?>
				<label for="data[FLAG_ANONYMOUS]"><?= tpl_text('Anonymous replies; do not save user information') ?></label><br />
			<?= tpl_form_checkbox('data[FLAG_PROMOTE]',$node->data['FLAG_PROMOTE']) ?>
				<label for="data[FLAG_PROMOTE]"><?= tpl_text('Display poll on frontpage') ?></label>
		</div>	

		<?= tpl_text('Date to enable poll') ?>
		<div class="indent">
			<input type="text" name="data[TS_START]" value="<?= tpl_date('Y-m-d', $node->data['TS_START']) ?>" 
				match="^(20[0-9][0-9])-(0[1-9]|1[0-2])-(0[1-9]|[1-2][0-9]|3[0-1])$|^$" message="<?= tpl_text('Invalid date') ?>" />
		</div>

		<?= tpl_text('Date to withdraw poll') ?>
		<div class="indent">
			<input type="text" name="data[TS_STOP]" value="<?= tpl_date('Y-m-d', $node->data['TS_STOP']) ?>" 
				match="^(20[0-9][0-9])-(0[1-9]|1[0-2])-(0[1-9]|[1-2][0-9]|3[0-1])$|^$" message="<?= tpl_text('Invalid date') ?>" />
		</div>
	</div>

	<input type="submit" name="post" value="<?= tpl_text('Save') ?>" />
</form>


<br /><br />
<div class="Abstract">
	<?= tpl_text("The permissions govern who get do what with this poll.") ?>
	<?= tpl_text("People need the 'Read' permission to reply to the poll.") ?>
</div>
<br />
<?= Module::runHook('permissions', $this, $node) ?>
