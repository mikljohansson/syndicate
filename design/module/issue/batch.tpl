<?
tpl_load_script(tpl_design_uri('js/json.js'));
tpl_load_script(tpl_design_uri('js/autocomplete.js'));
?>
<form class="Report" method="post">
	<input type="hidden" name="collections[]" value="<?= $collection->id() ?>" />
	<h1><?= tpl_text('Batch modify issues') ?></h1>
	
	<table>
		<tr class="<?= tpl_cycle(array('odd','even')) ?>">
			<th><?= tpl_text('Project') ?></th>
			<td>
				<select name="project">
					<option value="">&nbsp;</option>
					<? $this->iterate(SyndLib::sort($projects),'option_expand_children.tpl') ?>
				</select>
			</td>
			<th><?= tpl_text('Customer') ?></th>
			<td>
				<input type="text" name="customer" id="customer" size="45" />
				<script type="text/javascript">
				<!--
					if (document.getElementById) {
						window.onload = function() {
							new AutoComplete(document.getElementById('customer'), '<?= tpl_view('rpc','json','issue') ?>', 'findSuggestedUsers');
						};
					}
				//-->
				</script>
			</td>
		</tr>
		<tr class="<?= tpl_cycle() ?>">
			<th><?= tpl_text('Assigned') ?></th>
			<td>
				<? $options = SyndLib::invoke($handlers,'toString'); asort($options); ?>
				<select name="assigned">
					<option value="">&nbsp;</option>
					<option value="user_null.null"><?= tpl_text('Unassigned') ?></option>
					<?= tpl_form_options($options) ?>
				</select>
			</td>
			<th rowspan="4"><?= tpl_text('Categories') ?></th>
			<td rowspan="4">
				<select name="categories[]" multiple="multiple" size="6">
					<? $this->iterate(SyndLib::sort($categories), 'option_expand_keywords.tpl',array('selected' => array_flip((array)$request['keywords']))) ?>
				</select>
			</td>
		</tr>
		<tr class="<?= tpl_cycle() ?>">
			<th><?= tpl_text('Status') ?></th>
			<td>
				<select name="status">
					<option value="">&nbsp;</option>
					<?= tpl_form_options(array_map('tpl_text',$status_options),$request['status']) ?>
				</select>
			</td>
		</tr>
		<tr class="<?= tpl_cycle() ?>">
			<th><?= tpl_text('Priority') ?></th>
			<td>
				<select name="priority">
					<option value="">&nbsp;</option>
					<?= tpl_form_options(array_map(array($this,'text'),$priority_options),$request['priority']) ?>
				</select>
			</td>
		</tr>
		<tr class="<?= tpl_cycle() ?>">
			<th><?= tpl_text('Reschedule') ?></th>
			<td>
				<? $options = array(
					'+12 weeks' => tpl_text('%d months forward', 3),
					'+4 weeks'  => tpl_text('%d weeks forward', 4),
					'+2 weeks'  => tpl_text('%d weeks forward', 2),
					'+1 weeks'  => tpl_text('%d week forward', 1),
					0 => '&nbsp;',
					'-1 weeks'  => tpl_text('%d week backward', -1),
					'-2 weeks'  => tpl_text('%d weeks backward', -2),
					'-4 weeks'  => tpl_text('%d weeks backward', -4),
					'-12 weeks'  => tpl_text('%d months backward', -3));
				?>
				<select name="reschedule">
					<?= tpl_form_options($options,0) ?>
				</select>
			</td>
		</tr>
		<tr class="<?= tpl_cycle() ?>">
			<th><?= tpl_text('Fixed due date') ?></th>
			<td>
				<input type="text" name="date" title="<?= tpl_text('YYYY-MM-DD') ?>" size="21" /> 
			</td>
			<th><?= tpl_text('Options') ?></th>
			<td>
				<?= tpl_form_checkbox('delete',$request['delete']) ?>
				<label for="delete"><?= tpl_text('Delete all issue permanently') ?></label>
			</td>
		</tr>
	</table>
	
	<p>
		<input type="submit" name="post" value="<?= tpl_text('Apply') ?>" />
		<input type="button" value="<?= tpl_text('Abort') ?>" onclick="window.location='<?= tpl_uri_return() ?>';" />
	</p>
</form>

<br />
<? $this->display('model/node/issue/table.tpl', array('list'=>$collection->getContents(),'hideCheckbox'=>true)) ?>