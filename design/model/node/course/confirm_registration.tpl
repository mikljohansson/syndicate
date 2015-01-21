<? tpl_load_script(tpl_design_uri('js/form.js')) ?>
<div class="Article">
	<div class="Header">
		<h3><a href="<?= tpl_link($node->getHandler(),'view',$node->getCourseId()) ?>"><?= $node->getTitle() ?></a></h3>
	</div>
	<? if (null != $node->getDescription()) { ?>
	<div class="Abstract">
		<?= $node->getDescription() ?>
	</div>
	<? } ?>

	<br />
	<form method="post">
		<? $group = $node->getGroup(); ?>
		<h3><?= tpl_text('Please choose your group') ?></h3>
		<div class="indent Info">
			<?= tpl_text("Choosing a group allows you to receive group emails and makes it easy for the teacher to find your results and help you if need be. Select the %s group if unsure about which group you should belong to.", $group->toString()) ?>
		</div>
		<br />

		<div class="indent">
			<select name="group_node_id" match=".+" message="<?= tpl_text('Please select a group') ?>">
				<option value="">&nbsp;</option>
				<? $this->render($group,'option_expand_children.tpl') ?>
			</select>
		</div>
		<br />
		
		<input type="submit" value="<?= tpl_text('Register') ?>" />
		<input type="button" value="<?= tpl_text('Abort') ?>" onclick="window.location='<?= tpl_uri_return() ?>';" />
	</form>
</div>