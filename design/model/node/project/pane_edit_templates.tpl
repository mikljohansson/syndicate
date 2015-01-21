<? 
$module = Module::getInstance('issue');
$mailNotifier = $node->getMailNotifier();
$locale = $request[1];

?>
<? foreach ($module->getDefinedEvents($node) as $event => $description) { ?>
	<h3><?= tpl_text($event) ?></h3>
	<p class="Info"><?= $description ?> (<a href="<?= tpl_link_call($node->getHandler(),'invoke',$node->nodeId,'delTemplate',$event,$locale) ?>"><?= tpl_text('Reset this template to default') ?></a>)</p>
	<?= tpl_form_textarea("data[templates][$event][$locale]",$node->getTemplate($this, $event, $locale)) ?>
<? } ?>

<h3><?= tpl_text('Tags available for use in templates') ?></h3>
<dl class="Help">
	<dt>{$ID}</dt>
	<dd><?= tpl_text('Issue number') ?></dd>
	<dt>{$TITLE}</dt>
	<dd><?= tpl_text('Issue title field') ?></dd>
	<dt>{$DESCRIPTION}</dt>
	<dd><?= tpl_text('Description or email body') ?></dd>
	<dt>{$PROJECT}</dt>
	<dd><?= tpl_text('Name of project the issue resides in') ?></dd>
	<dt>{$ASSIGNED}</dt>
	<dd><?= tpl_text('Name of assigned user') ?></dd>
	<dt>{$ASSIGNED_CONTACT_INFO}</dt>
	<dd><?= tpl_text('Contact details of assigned user') ?></dd>
	<dt>{$CUSTOMER}</dt>
	<dd><?= tpl_text('Name of customer') ?></dd>
	<dt>{$CUSTOMER_CONTACT_INFO}</dt>
	<dd><?= tpl_text('Customer contact details') ?></dd>
	<dt>{$NOTE}</dt>
	<dd><?= tpl_text('Last added note, eg. the reply one send the customer') ?></dd>
	<dt>{$LINK}</dt>
	<dd><?= tpl_text('External link to issue') ?></dd>
	<dt>{$FEEDBACK_LINK_YES}</dt>
	<dd><?= tpl_text('Positive feedback link') ?></dd>
	<dt>{$FEEDBACK_LINK_NO}</dt>
	<dd><?= tpl_text('Negative feedback link') ?></dd>
</dl>
