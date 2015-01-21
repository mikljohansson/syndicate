<?
tpl_load_script(tpl_design_uri('js/json.js'));
tpl_load_script(tpl_design_uri('js/autocomplete.js'));

if (!isset($report)) {
	$request['state'] = 'active';
	$request['report'] = 'list';
	$request['recurse'] = '1';
}

?>
<h1><?= tpl_text('Leasing report') ?></h1>
<form class="Report" action="<?= tpl_link('inventory','leases') ?>" method="get">
	<? include tpl_design_path('gui/errors.tpl'); ?>
	<table>
		<tr class="<?= tpl_cycle(array('odd','even')) ?>">
			<th><?= tpl_text('Text filter') ?></th>
			<td><input type="text" name="query" value="<?= tpl_attribute($request['query']) ?>" size="45" /></td>
			<th><?= tpl_text('Costcenter') ?></th>
			<td><input type="text" name="costcenter" id="costcenter" value="<?= tpl_attribute($costcenter) ?>" size="45" /></td>
		</tr>
		<tr class="<?= tpl_cycle() ?>">
			<th><?= tpl_text('Folder') ?></th>
			<td>
				<? 
				if (null == ($folder = SyndNodeLib::getInstance($request['folder'])))
					$folder = SyndNodeLib::getInstance('null.null'); ?>
				<select name="folder">
					<option value="">&nbsp;</option>
					<? $this->iterate(SyndLib::sort($folders),'option_expand_children.tpl',array('selected'=>$folder)) ?>
				</select>
			</td>
			<th><?= tpl_text('Project') ?></th>
			<td><input type="text" name="project" id="project" value="<?= tpl_attribute($project) ?>" size="45" /></td>
		</tr>
		<tr class="<?= tpl_cycle() ?>">
			<th><?= tpl_text('Status') ?></th>
			<td>		
				<? $opt = array(null => '&nbsp;', 'terminated' => tpl_text('Terminated'), 'active' => tpl_text('Non-terminated')); ?>
				<select name="state">
					<?= tpl_form_options($opt,$request['state']) ?>
				</select>
			</td>
			<th><?= tpl_text('Customer') ?></th>
			<td><input type="text" name="customer" id="customer" value="<?= tpl_attribute($customer) ?>" size="45" /></td>
		</tr>
		<tr class="<?= tpl_cycle() ?>">
			<th><?= tpl_text('Created between') ?></th>
			<td>		
				<input type="text" name="created[0]" value="<?= tpl_attribute($request['created'][0]) ?>" size="10" /> <?= tpl_text('and') ?>
				<input type="text" name="created[1]" value="<?= tpl_attribute($request['created'][1]) ?>" size="10" />  <span class="Info"><?= tpl_text('(YYYY-MM-DD)') ?></span>
			</td>
			<th rowspan="2"><?= tpl_text('Options') ?></th>
			<td rowspan="2">
				<?= tpl_form_checkbox('empty',$request['empty']) ?> 
				<label for="empty"><?= tpl_text('Include empty leases') ?></label><br />
				<?= tpl_form_checkbox('recurse',$request['recurse']) ?>
				<label for="recurse"><?= tpl_text('Include all subfolders') ?></label><br />
			</td>
		</tr>
		<tr class="<?= tpl_cycle() ?>">
			<th><?= tpl_text('Expires between') ?></th>
			<td>		
				<input type="text" name="expires[0]" value="<?= tpl_attribute($request['expires'][0]) ?>" size="10" /> <?= tpl_text('and') ?>
				<input type="text" name="expires[1]" value="<?= tpl_attribute($request['expires'][1]) ?>" size="10" />  <span class="Info"><?= tpl_text('(YYYY-MM-DD)') ?></span>
			</td>
		</tr>
		<tr class="<?= tpl_cycle() ?>">
			<th>&nbsp;</th>
			<td>&nbsp;</td>
			<th><?= tpl_text('Report') ?></th>
			<td>
				<dl>
					<dt>
						<?= tpl_form_radiobutton('report',$request['report'],'list') ?>
						<label for="report[list]"><?= tpl_text('List leases and customers') ?></label>
					</dt>
					<dt>
						<?= tpl_form_radiobutton('report',$request['report'],'service') ?>
						<label for="report[service]"><?= tpl_text('Service level statistics') ?></label>
					</dt>
				</dl>
			</td>
		</tr>
	</table>
	<p>
		<input type="submit" name="output[html]" value="<?= tpl_text('Display') ?>" />
		<input type="submit" name="output[xls]" value="<?= tpl_text('Download (Excel)') ?>" />
	</p>
</form>

<? if (isset($report)) { ?>
<? $this->display(tpl_design_path("module/inventory/report/leases/formats/$format.html.tpl")) ?>
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
