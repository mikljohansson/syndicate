<?
require_once 'design/gui/PagedListing.class.inc';
require_once 'core/lib/Template.class.inc';

$query = $node->getAgreementsQuery();
$l = $query->join('synd_inv_lease', 'l');
if (!count($order = tpl_sort_order('lease',"$l.")))
	$order = array("$l.client_node_id");

$pager = new PagedListing($node->_storage, $node->getAgreementsQuery(), $order, 50);
$leases = $pager->getInstances();

?>
<div class="Article">
	<div class="Header">
		<h1><?= $node->toString() ?></h1>
		<? if ($node->isPermitted('write')) { ?>
		<ul class="Actions">
			<li><a href="<?= tpl_link_call('inventory','edit',$node->nodeId) ?>"><?= tpl_text('Edit this SLD') ?></a></li>
		</ul>
		<? } ?>
		<p><?= tpl_html_format($node->getDescription()) ?></p>
	</div>
	<? if (null != $node->data['INFO_URI']) { ?>
	<p><a href="<?= $node->data['INFO_URI'] ?>"><?= $node->data['INFO_URI'] ?></a></p>
	<? } ?>

	<? if (!empty($leases)) { ?>
		<div class="Result">
			<?= tpl_text('Displaying %d-%d of %d non-terminated leases', 
				$pager->getOffset()+1, $pager->getOffset()+count($leases), $pager->getCount()) ?><br />
			<? $this->display('gui/pager.tpl',$pager->getParameters()) ?>
		</div>
		<?= tpl_gui_table('lease',$leases,'view.tpl') ?>
		<? if ($pager->getCount() > $pager->getLimit()) { ?>
		<div class="Result">
			<? $this->display('gui/pager.tpl',$pager->getParameters()) ?>
		</div>
		<? } ?>
	<? } else { ?>
		<em><?= tpl_text('No active service level agreements.') ?></em>
	<? } ?>
</div>