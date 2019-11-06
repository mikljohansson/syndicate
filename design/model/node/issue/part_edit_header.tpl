<table class="Vertical">
	<tr class="<?= $this->cycle(array('odd','even')) ?>">
		<th><?= $this->text('Customer') ?></th>
		<td<? if(isset($errors['client'])) print ' class="InvalidField"'; ?>>
			<? if ($node->isPermitted('admin')) { ?>
				<? 
				if (null === ($customer = SyndNodeLib::getInstance($data['CLIENT_NODE_ID'])))
					$customer = $node->getCustomer(); 
				tpl_load_script(tpl_design_uri('js/autocomplete.js'));
				?>
				<input type="hidden" name="data[prevClient]" value="<?= tpl_attribute($data['client']) ?>" />
				<input tabindex="<?= $this->sequence() ?>" type="text" name="data[client]" id="client" value="<?= 
					tpl_value($data['client'], $customer->getLogin()) ?>" size="40" autocomplete="off" />
				<? if (isset($errors['client_matches'])) { ?>
					<div class="Info">
						<?= tpl_form_radiobutton('data[CLIENT_NODE_ID]',$data['CLIENT_NODE_ID'],"user_case.{$data['client']}") ?>
							<label for="data[CLIENT_NODE_ID][user_case.<?= tpl_attribute($data['client']) ?>]"><?= $this->text('Text <em>"%s"</em> only', $data['client']) ?></label><br />
						<? foreach (array_keys($clients = $errors['client_matches']) as $key) { ?>
						<?= tpl_form_radiobutton('data[CLIENT_NODE_ID]',$data['CLIENT_NODE_ID'],$clients[$key]->nodeId) ?>
							<label for="data[CLIENT_NODE_ID][<?= $clients[$key]->nodeId ?>]"><?= $clients[$key]->toString() ?><? 
								if (null != $clients[$key]->getContact()) { ?> (<?= $clients[$key]->getContact() ?>)<? } ?></label><br />
						<? } ?>
					</div>
				<? } ?>
				<script type="text/javascript">
				<!--
					if (document.getElementById) {
						window.onload = function() {
							new AutoComplete(document.getElementById('client'), '<?= tpl_view('rpc','json',$node->id()) ?>', 'findSuggestedUsers');
						};
					}

					// Toggles the issue status to RECENT, if it's currently PENDING
					function setAttentionStatus() {
						if (document.getElementById) {
							var pending = document.getElementById('data[status][<?= synd_node_issue::PENDING ?>]');
							var recent = document.getElementById('data[status][<?= synd_node_issue::RECENT ?>]');
							if (pending && recent && pending.checked) {
								recent.checked = true;
							}
						}
					}
				//-->
				</script>
			<? } else { ?>
			<? $this->render($node->getCustomer(),'contact.tpl',array('extended'=>true)); ?>
			<? } ?>
		</td>
		<th><?= $this->text('Due date') ?></th>
		<td<? if(isset($errors['TS_RESOLVE_BY'])) print ' class="InvalidField"'; ?>>
			<input type="text" name="data[TS_RESOLVE_BY]" value="<?= 
				tpl_date('Y-m-d', $data['TS_RESOLVE_BY'], $node->getParent()->getDefaultResolveBy()) ?>" size="40" 
				title="<?= $this->text('The date this issue should be resolved (YYYY-MM-DD)') ?>" 
				message="<?= $this->text('Invalid date') ?>" match="^(20[0-9][0-9])-(0[1-9]|1[0-2])-(0[1-9]|[1-2][0-9]|3[0-1])$" />
		</td>
	</tr>
	<tr class="<?= $this->cycle(array('odd','even')) ?>">	
		<th><?= $this->text('Project') ?></th>
		<td<? if(isset($errors['PARENT_NODE_ID'])) print ' class="InvalidField"'; ?>>
			<select tabindex="<?= $this->sequence() ?>" name="data[PARENT_NODE_ID]" onchange="setAttentionStatus(); Issue.loadProject('<?= 
				tpl_view('rpc','json') ?>', this, document.getElementById('data[ASSIGNED_NODE_ID]'));" onkeyup="setAttentionStatus(); Issue.loadProject('<?= 
				tpl_view('rpc','json') ?>', this, document.getElementById('data[ASSIGNED_NODE_ID]'));" onkeypress="Issue.findOption(this, event);" onmousewheel="return false;">
				<? $this->iterate(SyndLib::sort($node->getParentOptions()),
					'option_expand_children.tpl',array('selected'=>$node->_storage->getInstance($data['PARENT_NODE_ID']))) ?>
			</select>
		</td>
		<th rowspan="3"><?= $this->text('Status') ?></th>
		<td rowspan="3"><? $this->render($node,'part_edit_status.tpl',$_data) ?></td>
	</tr>
	<? if ($node->isPermitted('admin') || !$node->isNew()) { ?>
	<tr class="<?= $this->cycle() ?>">
		<th><?= $this->text('Assigned') ?></th>
		<td<? if(isset($errors['ASSIGNED_NODE_ID'])) print ' class="InvalidField"'; ?>>
			<? 
			if (null === ($assigned = SyndNodeLib::getInstance($data['ASSIGNED_NODE_ID'])))
				$assigned = $node->getAssigned(); 							

			$options = array();
			foreach (array_keys($users = $node->getAssignedOptions()) as $key) {
				$options[$key] = $users[$key]->toString();
				if (null != ($contact = $users[$key]->getContact()))
					$options[$key] .= " ($contact)";
			}
			uasort($options, 'strcasecmp');

			if ($node->isPermitted('admin')) { ?>
				<select tabindex="<?= $this->sequence() ?>" name="data[ASSIGNED_NODE_ID]" id="data[ASSIGNED_NODE_ID]" onchange="setAttentionStatus();" onkeyup="setAttentionStatus();" onmousewheel="return false;">
					<option value="user_null.null" class="Predefined"><?= $this->text('Unassigned') ?></option>
					<?= tpl_form_options($options,$assigned->nodeId) ?>
				</select>
			<? } else { ?>
				<? $this->render($assigned,'contact.tpl') ?>
			<? } ?>
		</td>
	</tr>
	<? } ?>
	<? if ($node->isPermitted('admin')) { ?>
	<tr class="<?= $this->cycle() ?>">
		<th><?= $this->text('Priority') ?></th>
		<td<? if(isset($errors['INFO_PRIO'])) print ' class="InvalidField"'; ?>>
			<select tabindex="<?= $this->sequence() ?>" name="data[INFO_PRIO]" onmousewheel="return false;">
				<?= tpl_form_options(array_map(array($this,'text'),$node->getDefinedPriorities()),$data['INFO_PRIO']) ?>
			</select>
		</td>
	</tr>
	<? } ?>					
</table>
