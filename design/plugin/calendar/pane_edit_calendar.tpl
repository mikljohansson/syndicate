<?
global $synd_config;
require_once 'core/lib/SyndDate.class.inc';
require_once 'design/gui/CalendarWidget.class.inc';
tpl_load_script(tpl_design_uri('js/calendar.js'), false);

if (!isset($data['calendar']['assigned'])) {
	if (isset($request['selected']))
		$data['calendar']['assigned'] = $request['selected'];
	else {
		$user = $node->getAssigned();
		$data['calendar']['assigned'] = $user->nodeId;
	}
}

$calendar = $plugin->getCalendar();
$users = $plugin->getSelectedUsers($node, $data['calendar']);

$timestamp = isset($request[2]) && is_numeric($request[2]) ? $request[2] : time();
$start = strtotime('08:00', SyndDate::startOfWeek($timestamp));
$end = strtotime('-2 days 17:00', SyndDate::endOfWeek($timestamp));
$widget = new CalendarWeekWidget($start, $end, 1800);
$colors = array();

$seen = array();
$found = array();
$eventids = (array)$node->getAttribute('eventids');

foreach ($users as $key => $user) {
	$colors[$key] = tpl_cycle(array('EventType1','EventType2','EventType3'),'colors');
	
	try {
		if (null !== ($agenda = $calendar->getAgenda($user))) {
			foreach ($agenda->getEventsByRange($start, $end) as $event) {
				if (!in_array($id = $event->getProperty('UID'), $seen)) {
					if (isset($eventids[$id]))
						$seen[] = $id;
					$widget->addEvent(new CalendarEventAdapter($this, $node, $event, 
						array('class' => 'Event '.(isset($eventids[$id])?'ConfirmedEvent':$colors[$key])), 
						isset($eventids[$id])));
				}
			}
			$found[$key] = $user;
		}
	}
	catch (RuntimeException $e) {
	
	}
}

// Add tentative non-committed events
if (isset($node->_calendar)) {
	foreach ($node->_calendar as $event)
		$widget->addEvent(new CalendarEventAdapter($this, $node, $event, array('class' => 'Event TentativeEvent'), true));
}

$table = $widget->getTable();
$table->setAttribute('id', $id = md5(uniqid('')));
$table->setDefaultAttributesCallback(array($widget, '_callback_javascript_attributes'));

?>
<table class="Vertical">
	<tr>
		<th style="width:100%;">
			<? if (empty($found)) { ?>
				<div class="Notice">
					<?= tpl_text("No calendar agenda found for any of the selected users, please contact your systems administrator if the problem persists.") ?>
				</div>
			<? } else { ?>
				<?= $table->toString() ?>
			<? } ?>
			<script type="text/javascript">
			<!--
				function _padInteger(num, width, padding) {
					str = num.toString();
					while (str.length < width)
						str = padding+str;
					return str;
				}

				if (document.getElementById) {
					var oCalendar = new Calendar(document.getElementById('<?= $id ?>'));
					oCalendar.setFinishedCallback(function(oCalendarEvent) {
							var oStart = oCalendarEvent.getStart(), oEnd = oCalendarEvent.getEnd();
							document.getElementById('data[calendar][date]').value = oStart.getFullYear()+'-'+_padInteger(oStart.getMonth()+1,2,'0')+'-'+_padInteger(oStart.getDate(),2,'0');
							document.getElementById('data[calendar][from]').value = _padInteger(oStart.getHours(),2,'0')+':'+_padInteger(oStart.getMinutes(),2,'0');
							document.getElementById('data[calendar][to]').value = _padInteger(oEnd.getHours(),2,'0')+':'+_padInteger(oEnd.getMinutes(),2,'0')
							document.getElementById('data[calendar][to]').form.submit();
						});
				}
			//-->
			</script>
		</th>
	</tr>
	<tr>
		<th style="width:100%;">
			<table>
				<tr>
					<td>
					<? foreach (array_keys($found) as $key) { ?>
						<? if (!$found[$key]->isNull()) { ?>
						<span class="<?= $colors[$key] ?>"><img src="<?= tpl_design_uri('image/pixel.gif') ?>" alt=" " /></span> <?= $found[$key]->toString() ?>&nbsp;
						<? } ?>
					<? } ?>
					</td>
					<td>
						<?= tpl_text('Display user') ?>
						<select name="data[calendar][assigned]" id="data[calendar][assigned]" onchange="this.form.submit();">
							<?= tpl_form_options(SyndLib::invoke(SyndLib::sort($node->getAssignedOptions()),'toString'),$data['calendar']['assigned']) ?>
						</select>
					</td>
					<td>
						<a href="<?= tpl_link_jump($node->getHandler(),'edit',$node->nodeId,'calendar',strtotime('-4 week',$timestamp),
							array('selected'=>$data['calendar']['assigned'])) ?>" title="<?= tpl_text('Skip backwards %d weeks',4) ?>">&laquo;&laquo;</a>
						<a href="<?= tpl_link_jump($node->getHandler(),'edit',$node->nodeId,'calendar',strtotime('-1 week',$timestamp),
							array('selected'=>$data['calendar']['assigned'])) ?>" title="<?= tpl_text('Skip backwards %d week',1) ?>">&laquo;</a>
						<?= tpl_text('Week %s', date('W',$timestamp)) ?>
						<a href="<?= tpl_link_jump($node->getHandler(),'edit',$node->nodeId,'calendar',strtotime('+1 week',$timestamp),
							array('selected'=>$data['calendar']['assigned'])) ?>" title="<?= tpl_text('Skip forward %d week',1) ?>">&raquo;</a>
						<a href="<?= tpl_link_jump($node->getHandler(),'edit',$node->nodeId,'calendar',strtotime('+4 week',$timestamp),
							array('selected'=>$data['calendar']['assigned'])) ?>" title="<?= tpl_text('Skip forward %d weeks',4) ?>">&raquo;&raquo;</a>
					</td>
				</tr>
				<? if (!empty($found)) { ?>
				<tr>
					<td colspan="3">
						<fieldset>
							<legend><?= tpl_text('Schedule meeting') ?></legend>
							<?= tpl_text('Date') ?> <input type="text" name="data[calendar][date]" id="data[calendar][date]" size="10" maxlength="10" />&nbsp;&nbsp;
							
							<?= tpl_text('From') ?>
							<input type="text" name="data[calendar][from]" id="data[calendar][from]" size="5" maxlength="5" /> <?= tpl_text('to') ?>
							<input type="text" name="data[calendar][to]" id="data[calendar][to]" size="5" maxlength="5" />&nbsp;&nbsp;
						
							<input type="submit" value="<?= tpl_text('Add') ?>" />
						</fieldset>
					</td>
				</tr>
				<? } ?>
			</table>
		</th>
	</tr>
</table>