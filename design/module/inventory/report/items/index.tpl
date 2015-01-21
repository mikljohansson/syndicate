<?
tpl_load_script(tpl_design_uri('js/json.js'));
tpl_load_script(tpl_design_uri('js/autocomplete.js'));

if (!isset($report)) {
	$request['report'] = 'list';
	$request['recurse'] = '1';
}

?>
<h1><?= tpl_text('Items report') ?></h1>
<form class="Report" action="<?= tpl_link('inventory','items') ?>" method="get">
	<? include tpl_design_path('gui/errors.tpl'); ?>
	<table>
		<tr class="<?= tpl_cycle(array('odd','even')) ?>">
			<th><?= tpl_text('Text filter') ?></th>
			<td><input type="text" name="query" value="<?= tpl_attribute($request['query']) ?>" size="45" /></td>
			<th><?= tpl_text('Costcenter') ?></th>
			<td><input type="text" name="costcenter" id="costcenter" value="<?= tpl_attribute($costcenter) ?>" size="45" /></td>
		</tr>
		<tr class="<?= tpl_cycle() ?>">
			<th><?= tpl_text('Category') ?></th>
			<td>
				<select name="class">
					<option value="">&nbsp;</option>
					<? $this->iterate(SyndLib::sort($classes),'option.tpl',array('selected'=>$class)) ?>
				</select>
			</td>
			<th><?= tpl_text('Project') ?></th>
			<td><input type="text" name="project" id="project" value="<?= tpl_attribute($project) ?>" size="45" /></td>
		</tr>
		<tr class="<?= tpl_cycle() ?>">
			<th rowspan="3"><?= tpl_text('Folder') ?></th>
			<td rowspan="3">
				<select name="folders[]" multiple="multiple" size="11">
					<? $this->iterate(SyndLib::sort($folders),'option_expand_children.tpl',array('selected'=>$selfolders)) ?>
				</select>
			</td>
			<th><?= tpl_text('Customer') ?></th>
			<td><input type="text" name="customer" id="customer" value="<?= tpl_attribute($customer) ?>" size="45" /></td>
		</tr>
		<tr class="<?= tpl_cycle() ?>">
			<th><?= tpl_text('Options') ?></th>
			<td>
				<?= tpl_form_checkbox('recurse',$request['recurse']) ?>
				<label for="recurse"><?= tpl_text('Include all subfolders') ?></label><br />
			</td>
		</tr>
		<tr class="<?= tpl_cycle() ?>">
			<th><?= tpl_text('Report') ?></th>
			<td>
				<dl>
					<dt>
						<?= tpl_form_radiobutton('report',$request['report'],'list') ?>
						<label for="report[list]"><?= tpl_text('List items') ?></label>
					</dt>
					<dd><?= tpl_text('Displays items with make, model, serial and other info.') ?></dd>
					<dt>
						<?= tpl_form_radiobutton('report',$request['report'],'trends') ?>
						<label for="report[trends]"><?= tpl_text('Item availability trends') ?></label>
					</dt>
					<dd><?= tpl_text('Tries to predict the number of available items of each model at a given date') ?></dd>
				</dl>
			</td>
		</tr>
		<tr class="<?= tpl_cycle() ?>">
			<th><?= tpl_text('Updated between') ?></th>
			<td>		
				<input type="text" name="updated[0]" value="<?= tpl_attribute($request['updated'][0]) ?>" size="10" /> <?= tpl_text('and') ?>
				<input type="text" name="updated[1]" value="<?= tpl_attribute($request['updated'][1]) ?>" size="10" />  <span class="Info"><?= tpl_text('(YYYY-MM-DD)') ?></span>
			</td>
			<th><?= tpl_text('Report date') ?></th>
			<td title="<?= tpl_text('YYYY-MM-DD, simplified formats such as \'-2 hours\' are supported') ?>">
				<input type="text" name="date" value="<?= tpl_attribute($request['date']) ?>" size="10" />
			</td>
		</tr>
	</table>
	<p>
		<input type="submit" name="output[html]" value="<?= tpl_text('Display') ?>" />
		<input type="submit" name="output[xls]" value="<?= tpl_text('Download (Excel)') ?>" />
	</p>
</form>

<? if (isset($report)) { ?>
<? $this->display(tpl_design_path("module/inventory/report/items/formats/$format.html.tpl")) ?>
<? } ?>

<script type="text/javascript">
<!--
	if (document.getElementById) {
		window.onload = function() {
			new AutoComplete(document.getElementById('costcenter'), '<?= tpl_view('rpc','json','inventory') ?>', 'findSuggestedCostcenters');
			new AutoComplete(document.getElementById('project'), '<?= tpl_view('rpc','json','inventory') ?>', 'findSuggestedProjects');
			new AutoComplete(document.getElementById('customer'), '<?= tpl_view('rpc','json','inventory') ?>', 'findSuggestedUsers');
		};
	}
//-->
</script>
