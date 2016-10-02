<? 
require_once 'core/lib/SyndHTML.class.inc';
$customer = $node->getCustomer(); 
$info = '';
if (null != $node->getCreated())
	$info .= tpl_text('Valid from %s', date('Y-m-d', $node->getCreated()));
if (null != $node->getExpire())
	$info .= ' '.tpl_text('to %s', date('Y-m-d', $node->getExpire()));
?>
<div class="SearchResult">
	<h3><img src="<?= tpl_design_uri('image/icon/16x16/lease.gif') ?>" alt="" height="16" width="16" /> <a href="<?= tpl_link('user','summary',$customer->nodeId) ?>"><?= synd_htmlspecialchars($node->getTitle()) ?></a></h3>
	<? if ('' != ($info = trim($info, ', '))) { ?><div class="Info"><?= ucfirst($info) ?></div><? } ?>
	<div class="Abstract">
		<?= SyndHTML::getContextSummary($node->getDescription(), $highlight, 200) ?>
		<? if (count($items = $node->getItems())) { ?>
		<ul class="Actions">
			<? foreach (array_slice(array_keys($items),0,2) as $key) { ?>
			<li><a href="<?= tpl_link($items[$key]->getHandler(),'view',$items[$key]->nodeId) ?>"><?= $items[$key]->toString() ?></a></li>
			<? } ?>
		</ul>
		<? } ?>
	</div>
	<div class="Footer"><?= tpl_request_host() ?><?= tpl_link('user','summary',$customer->nodeId) ?></div>
</div>
