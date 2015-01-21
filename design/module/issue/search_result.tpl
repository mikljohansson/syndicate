<div class="Notice">
	<em><?= tpl_translate("You are not authorized to view the issue <b>#%d</b> belonging to <b>%s</b> (%s%s), please contact the projects administrator if the problem persists.", 
		tpl_quote($issue->objectId()), tpl_quote($issue->getParent()->toString()), 
		$issue->getParent()->getEmail() ? tpl_email($issue->getParent()->getEmail()).', ' : '',
		tpl_quote(tpl_chop($issue->getParent()->getDescription(),50))) ?></em>
</div>
