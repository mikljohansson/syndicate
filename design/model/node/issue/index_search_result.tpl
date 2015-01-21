<? 
require_once 'core/lib/SyndHTML.class.inc';
$customer = $node->getCustomer(); 
$project = $node->getParent(); 
?>
<div class="SearchResult">
	<h3><a href="<?= tpl_link($node->getHandler(),$node->objectId()) ?>"><?= htmlspecialchars($node->getTitle()) ?></a></h3>
	<div class="Info"><?= tpl_translate('From <a href="%s">%s</a>, project <a href="%s">%s</a>, last modified %s', 
		tpl_link('user','summary',$customer->nodeId), $customer->toString(), 
		tpl_link('issue','project',$project->getProjectId(),'issues'), 
		tpl_quote($project->toString()), 
		tpl_quote(date('Y-m-d', $node->data['TS_UPDATE']))) ?></div>
	<div class="Abstract"><?= SyndHTML::getContextSummary(wordwrap(htmlspecialchars($node->getDescription()),70,' '), $highlight, 300) ?></div>
	<div class="Footer"><?= tpl_request_host() ?><?= tpl_link($node->getHandler(),$node->objectId()) ?></div>
</div>
