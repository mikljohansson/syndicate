<? $inventory = Module::getInstance('inventory'); ?>
<form action="<?= tpl_view_call('inventory','setFolders') ?>" method="post">
	<ul class="Actions">
		<li><a href="<?= tpl_link_call('inventory','newFolder') ?>"><?= tpl_text('Create new top-level folder') ?></a></li>
	</ul>
	
	<h2><?= tpl_text('Settings for inventory routines.') ?></h2>
	<table class="indent" style="margin-bottom:1em;">
		<tr>
			<td>
				<select name="repair_project">
					<option value="">&nbsp;</option>
					<? $this->iterate($inventory->getProjects(),'option_expand_children.tpl',
						array('selected' => $inventory->getRepairProject())) ?>
				</select>
			</td>
			<td><?= tpl_text('Project to assign issues to.') ?></td>
		</tr>
		<tr>
			<td>
				<select name="invoice_project">
					<option value="">&nbsp;</option>
					<? $this->iterate($inventory->getProjects(),'option_expand_children.tpl',
						array('selected' => $inventory->getInvoiceProject())) ?>
				</select>
			</td>
			<td><?= tpl_text('Project to handle invoice issues.') ?></td>
		</tr>
		<tr>
			<td>
				<select name="repair_folder">
					<option value="">&nbsp;</option>
					<? $this->display(tpl_design_path('module/inventory/folder_options.tpl'),
					array('selected' => $inventory->getRepairFolder())) ?>
				</select>
			</td>
			<td><?= tpl_text('Folder to move broken items to after replacing them.') ?></td>
		</tr>
		<tr>
			<td>
				<select name="repaired_folder">
					<option value="">&nbsp;</option>
					<? $this->display(tpl_design_path('module/inventory/folder_options.tpl'),
					array('selected' => $inventory->getRepairedFolder())) ?>
				</select>
			</td>
			<td><?= tpl_text('Folder to move items to when back from repairs.') ?></td>
		</tr>
		<tr>
			<td>
				<select name="leased_folder">
					<option value="">&nbsp;</option>
					<? $this->display(tpl_design_path('module/inventory/folder_options.tpl'),
					array('selected' => $inventory->getLeasedFolder())) ?>
				</select>
			</td>
			<td><?= tpl_text('Folder to move item to after being handed out.') ?></td>
		</tr>
		<tr>
			<td>
				<select name="terminate_folder">
					<option value="">&nbsp;</option>
					<? $this->display(tpl_design_path('module/inventory/folder_options.tpl'),
						array('selected' => $inventory->getTerminateFolder())) ?>
				</select>
			</td>
			<td><?= tpl_text('Folder to place items in after terminating a lease.') ?></td>
		</tr>
		<tr>
			<td>
				<select name="terminated_lease_folder">
					<option value="">&nbsp;</option>
					<? $this->display(tpl_design_path('module/inventory/folder_options.tpl'),
						array('selected' => $inventory->getTerminatedLeaseFolder())) ?>
				</select>
			</td>
			<td><?= tpl_text('Folder to place leases after being terminated.') ?></td>
		</tr>
		<tr>
			<td>
				<select name="agent_folder">
					<option value="">&nbsp;</option>
					<? $this->display(tpl_design_path('module/inventory/folder_options.tpl'),
						array('selected' => $inventory->getAgentFolder())) ?>
				</select>
			</td>
			<td><?= tpl_text('Folder for items the inventory agent finds.') ?></td>
		</tr>
		<tr>
			<td>
				<select name="lease_folder">
					<option value="">&nbsp;</option>
					<? $this->display(tpl_design_path('module/inventory/folder_options.tpl'),
						array('selected' => $inventory->getLeaseFolder())) ?>
				</select>
			</td>
			<td><?= tpl_text('Default folder for new leases.') ?></td>
		</tr>
		<tr>
			<td>
				<select name="sold_folder">
					<option value="">&nbsp;</option>
					<? $this->display(tpl_design_path('module/inventory/folder_options.tpl'),
						array('selected' => $inventory->getSoldFolder())) ?>
				</select>
			</td>
			<td><?= tpl_text('Folder for sold items.') ?></td>
		</tr>
	</table>

	<input type="submit" value="<?= tpl_text('Save') ?>" />
</form>
