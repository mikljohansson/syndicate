<? 
if (isset($_REQUEST['mailed'])) {
	$formatted = true;
	foreach ((array)$_REQUEST['mailed'] as $eventid => $listenerids) {
		if (is_numeric($eventid)) {
			if ('' != $emails) $emails .= ', ';
			$emails .= tpl_quote($listenerids);
			$formatted = false;
		}
		else if (null !== ($event = SyndLib::getInstance($eventid)) && null !== ($issue = $event->getParent())) {
			foreach (array_keys($listeners = SyndLib::getInstances($listenerids)) as $key) {
				if ('' != $emails) $emails .= ', ';
				$emails .= '<a href="'.tpl_link('issue','view',$issue->nodeId,'email',array('event'=>$eventid,'listeners'=>$listenerids)).'">'.tpl_quote($listeners[$key]->toString()).'</a>';
			}
		}
	}
	
	if (null != $emails) { ?>
		<div class="Success">
			<?= $formatted ? tpl_translate("Sent email to <em>%s</em>, click to view the email sent", $emails) : tpl_translate("Sent email to <em>%s</em>", $emails) ?>
		</div><? 
	} 
}
