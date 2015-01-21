<form method="post">
	<input type="hidden" name="collections[]" value="<?= $collection->id() ?>" />
	<div class="Dialogue">
		<h1><?= tpl_text('Move items to folder') ?></h1>
		<table>
			<tr>
				<td>
					<h4><?= tpl_text('Folder') ?></h4>
					<select name="folder">
						<option value="">&nbsp;</option>
						<? $this->display(tpl_design_path('module/inventory/folder_options.tpl')); ?>
					</select>
				</td>
				<td>
					<h4><?= tpl_text('Print receipt') ?></h4>
					<select name="receipt">
						<option value="">&nbsp;</option>
						<? $this->iterate($module->getTemplates('move'),'list_view_option.tpl') ?>
					</select>
				</td>
			</tr>
		</table>
		<p>
			<input type="submit" name="confirm" value="<?= tpl_text('Ok') ?>" />
			<input type="button" value="<?= tpl_text('Abort') ?>" onclick="window.location='<?= tpl_uri_return() ?>';" />
		</p>
	</div>
</form>

<h3><?= tpl_text('Items to be moved') ?></h3>
<?= tpl_gui_table('item',$items,'view.tpl',array('hideCheckbox'=>true)) ?>
