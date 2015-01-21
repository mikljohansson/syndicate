<? 
$customer = $node->getCustomer();
$folder = $node->getParent();
$info = '';

if (!$customer->isNull()) 
	$info .= tpl_translate('User %s', $this->fetchnode($customer,'head_view.tpl'));
if (!$folder->isNull()) 
	$info .= ', '.tpl_translate('folder <a href="%s">%s</a>', tpl_link($folder->getHandler(),'view',$folder->nodeId), tpl_quote($folder->toString()));
if (null != $node->data['INFO_SERIAL_MAKER'])
	$info .= ', '.tpl_text('serial number <em>%s</em>', $node->data['INFO_SERIAL_MAKER']);

?>
<div class="SearchResult">
	<h3><img src="<?= tpl_design_uri('image/icon/16x16/computer.gif') ?>" alt="" height="16" width="16" /> <a href="<?= tpl_link($node->getHandler(),'view',$node->nodeId) ?>"><?= htmlspecialchars($node->getTitle()) ?></a></h3>
	<? if ('' != ($info = trim($info, ', '))) { ?><div class="Info"><?= ucfirst($info) ?></div><? } ?>
	<div class="Abstract"></div>
	<div class="Footer"><?= tpl_request_host() ?><?= tpl_link($node->getHandler(),'view',$node->nodeId) ?></div>
</div>
