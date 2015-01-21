<? require_once 'core/lib/SyndHTML.class.inc'; ?>
<div class="Item">
	<div class="Info">
		<?= ucwords(tpl_strftime('%A, %d %B %Y %H:%M', $node->data['TS_CREATE'])) ?>
	</div>
	<div class="Body">
		<? $parent = $node->getParent(); ?>
		<a href="<?= tpl_link($parent->getHandler(),'view',$parent->nodeId) ?>">
		<?= strip_tags(tpl_chop($node->data['INFO_BODY'],100)) ?></a>
	</div>
</div>