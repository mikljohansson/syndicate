<?
$assigned = array();
foreach ($handlers as $user)
	$assigned[$user->nodeId] = $user->toString().' ('.$user->getContact().')';
uasort($assigned, 'strcasecmp');

?>
<script type="text/javascript">
<!--
function indentcount(t) {
	return t.length - t.replace(/^\s*/, "").length;
}

function toggle_category_options(select, ev) {
	var found = false;
	var ic = indentcount(ev.target.text);
	
	for (var i = 0; i < select.options.length; i++) {
		option = select.options[i]
		
		if (found) {
			if (indentcount(option.text) > ic) {
				option.selected = ev.target.selected;
			}
			else {
				break;
			}
		}
		else if (option == ev.target)
			found = true;
	}
}
-->
</script>
<table>
	<tr class="<?= tpl_cycle(array('odd','even')) ?>">
		<th><?= tpl_text('Project') ?></th>
		<td>
			<? 
			if (null == ($project = SyndNodeLib::getInstance($request['project'])))
				$project = SyndNodeLib::getInstance('null.null'); ?>
			<select name="project" onchange="Issue.loadProject('<?= tpl_view('rpc','json') 
				?>', this, document.getElementById('assigned'));" onkeyup="Issue.loadProject('<?= tpl_view('rpc','json') 
				?>', this, document.getElementById('assigned'));"  onkeypress="Issue.findOption(this, event);" onmousewheel="return false;">
				<option value="">&nbsp;</option>
				<? $this->iterate(SyndLib::sort($projects),'option_expand_children.tpl',array('selected'=>$project)) ?>
			</select>
		</td>
		<th><?= tpl_text('Organisation') ?></th>
		<td><input type="text" name="organisation" id="organisation" value="<?= tpl_attribute($organisation) ?>" size="45" /></td>
	</tr>
	<tr class="<?= tpl_cycle() ?>">
		<th><?= tpl_text('Assigned') ?></th>
		<td>
			<select name="assigned" id="assigned">
				<?= tpl_form_options(array(
					''				=> '&nbsp;',
					'unassigned'	=> tpl_text('Unassigned'),
					'assigned'		=> tpl_text('Assigned')), 
					$request['assigned'], array('class' => 'Predefined')) ?>
				<?= tpl_form_options($assigned, $request['assigned']) ?>
			</select>
		</td>
		<th><?= tpl_text('Customer') ?></th>
		<td><input type="text" name="customer" id="customer" value="<?= tpl_attribute($customer) ?>" size="45" /></td>
	</tr>
	<tr class="<?= tpl_cycle() ?>">
		<th><?= tpl_text('Status') ?></th>
		<td>
			<? 
			$options = array(
				'all' 		=> tpl_text('All'),
				'open'		=> tpl_text('Open'),
				'closed'	=> tpl_text('Closed'),
				'overdue'	=> tpl_text('Overdue'));
			?>
			<select name="status">
				<optgroup label="<?= tpl_text('Status range') ?>">
					<?= tpl_form_options($options,empty($request['status'])?'open':$request['status']) ?>
				</optgroup>
				<optgroup label="<?= tpl_text('Specific status') ?>">
					<?= tpl_form_options(array_map('tpl_text',$status_options),$request['status']) ?>
				</optgroup>
			</select>
		</td>
		<th rowspan="7"><?= tpl_text('Categories') ?></th>
		<td rowspan="7">
			<select name="keywords[]" multiple="multiple" size="10" onclick="toggle_category_options(this, event)">
				<? $this->iterate(SyndLib::sort($categories), 'option_expand_keywords.tpl',array('selected' => array_flip((array)$request['keywords']))) ?>
			</select>
		</td>
	</tr>
	<tr class="<?= tpl_cycle() ?>">
		<th><?= tpl_text('Min priority') ?></th>
		<td>
			<select name="priority">
				<option value="">&nbsp;</option>
				<?= tpl_form_options(array_map(array($this,'text'),$priority_options),$request['priority']) ?>
			</select>
		</td>
	</tr>
	<tr class="<?= tpl_cycle() ?>">
		<th><?= tpl_text('Created between') ?></th>
		<td title="<?= tpl_text('YYYY-MM-DD, simplified formats such as \'-2 hours\' are supported') ?>">
			<input type="text" name="created[0]" value="<?= tpl_attribute($request['created'][0]) ?>" size="10" /> <?= tpl_text('and') ?>
			<input type="text" name="created[1]" value="<?= tpl_attribute($request['created'][1]) ?>" size="10" /> 
		</td>
	</tr>
	<tr class="<?= tpl_cycle() ?>">
		<th><?= tpl_text('Updated between') ?></th>
		<td title="<?= tpl_text('YYYY-MM-DD, simplified formats such as \'-2 hours\' are supported') ?>">
			<input type="text" name="updated[0]" value="<?= tpl_attribute($request['updated'][0]) ?>" size="10" /> <?= tpl_text('and') ?>
			<input type="text" name="updated[1]" value="<?= tpl_attribute($request['updated'][1]) ?>" size="10" /> 
		</td>
	</tr>
	<tr class="<?= tpl_cycle() ?>">
		<th><?= tpl_text('Closed between') ?></th>
		<td title="<?= tpl_text('YYYY-MM-DD, simplified formats such as \'-2 hours\' are supported') ?>">
			<input type="text" name="closed[0]" value="<?= tpl_attribute($request['closed'][0]) ?>" size="10" /> <?= tpl_text('and') ?>
			<input type="text" name="closed[1]" value="<?= tpl_attribute($request['closed'][1]) ?>" size="10" /> 
		</td>
	</tr>
	<tr class="<?= tpl_cycle() ?>">
		<th><?= tpl_text('Due between') ?></th>
		<td title="<?= tpl_text('YYYY-MM-DD, simplified formats such as \'-2 hours\' are supported') ?>">
			<input type="text" name="due[0]" value="<?= tpl_attribute($request['due'][0]) ?>" size="10" /> <?= tpl_text('and') ?>
			<input type="text" name="due[1]" value="<?= tpl_attribute($request['due'][1]) ?>" size="10" /> 
		</td>
	</tr>
	<tr class="<?= tpl_cycle() ?>">
		<th><?= tpl_text('Timecard interval') ?></th>
		<td title="<?= tpl_text('YYYY-MM-DD, simplified formats such as \'-2 hours\' are supported') ?>">
			<input type="text" name="interval[0]" value="<?= tpl_attribute($request['interval'][0]) ?>" size="10" /> <?= tpl_text('and') ?>
			<input type="text" name="interval[1]" value="<?= tpl_attribute($request['interval'][1]) ?>" size="10" /> 
		</td>
	</tr>
</table>
