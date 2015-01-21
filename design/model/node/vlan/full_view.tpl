<h1><?= $node->toString() ?></h1>
<ul class="Actions">
	<li><a href="<?= tpl_link_call($node->getHandler(),'edit',$node->nodeId) ?>"><?= tpl_text('Edit this VLAN') ?></a></li>
</ul>
<p><?= tpl_html_format($node->getDescription()) ?></p>

<h3><?= tpl_text('Networks assigned to this VLAN') ?></h3>
<? if (count($node->getNetworks())) { ?>
	<table>
		<thead>
			<th><?= tpl_text('Network address') ?></th>
			<th><?= tpl_text('Netmask') ?></td>
		</thead>
		<tbody>
		<? foreach ($node->getNetworks() as $network) { ?>
		<tr class="<?= tpl_cycle(array('odd','even')) ?>">
			<td><?= $network['INFO_NETWORK_ADDRESS'] ?></td>
			<td><?= $network['INFO_NETWORK_MASK'] ?></td>
		</tr>
		<? } ?>
		</tbody>
	</table>
<? } else { ?>
	<p><em><?= tpl_text('No networks assigned to this VLAN') ?></em></p>
<? } ?>

<? if (count($node->getAvailableRanges())) { ?>
	<h3><?= tpl_text('Unallocated IP-ranges') ?></h3>
	<table>
		<thead>
			<tr>
				<th><?= tpl_text('From') ?></th>
				<th><?= tpl_text('To') ?></th>
			</tr>
		</thead>
		<tbody>
			<? foreach ($node->getAvailableRanges() as $range) { ?>
			<tr class="<?= tpl_cycle(array('odd','even')) ?>">
				<td><?= long2ip($range[0]) ?></td>
				<td><?= long2ip($range[1]) ?></td>
			</tr>
			<? } ?>
		</tbody>
	</table>
<? } else { ?>
	<p><em><?= tpl_text('No IP-addresses available') ?></em></p>
<? } ?>

<? 
$listing = $node->getInterfacesListing();
$interfaces = $listing->getInstances();

if ($listing->getCount()) { ?>
	<div class="Result">
		<table style="width:100%;">
			<tr>
				<td>
					<?= tpl_text('Results %d-%d of %d devices matching these networks', 
						$listing->getOffset()+1, $listing->getOffset()+count($interfaces), $listing->getCount()) ?>
					<? $this->display('gui/pager.tpl',$listing->getParameters()) ?>
				</td>
				<td style="text-align:right; vertical-align:bottom;">
					<? include tpl_design_path('model/node/item/item_status_legend.tpl') ?>
				</td>
			</tr>
		</table>
	</div>
	<?= tpl_gui_table('interface',$interfaces,'view.tpl') ?>
<? } else { ?>
	<p><em><?= tpl_text('No devices matching these networks') ?></em></p>
<? } ?>