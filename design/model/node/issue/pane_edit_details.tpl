		<? 
		global $synd_user; 
		require_once 'core/lib/SyndDate.class.inc';
		?>
		<table class="Vertical">
			<tr>
				<th><?= $this->text('Time estimate') ?></th>
				<td<? if(isset($errors['INFO_ESTIMATE'])) print ' class="InvalidField"'; ?>>
					<input type="text" tabindex="<?= $this->sequence() ?>" name="data[INFO_ESTIMATE]" value="<?= tpl_value(SyndDate::durationExpr($data['INFO_ESTIMATE'])) ?>" style="width:200px;" /> (<?= $this->text('Minutes') ?>)
				</td>
			</tr>
			<?= SyndLib::runHook('issue_edit_details', $node, $_data) ?>
			<tr>
				<th><?= $this->text('E-mail me') ?></th>
				<td>
					<input type="hidden" name="data[flags]" value="1" />
					<? $mailNotifier = $node->getMailNotifier(); ?>
					<?= tpl_form_checkbox("data[notify][onresolve]",$mailNotifier->isRegisteredPermanent('onresolve',$synd_user),1,null,array('tabindex'=>$this->sequence())) ?> 
						<label for="data[notify][onresolve]"><?= $this->text('When issue is closed') ?></label><br />
					<?= tpl_form_checkbox("data[notify][onstart]",$mailNotifier->isRegisteredPermanent('onstart',$synd_user),1,null,array('tabindex'=>$this->sequence())) ?> 
						<label for="data[notify][onstart]"><?= $this->text('When work is begun') ?></label><br />
					<?= tpl_form_checkbox("data[notify][onchange]",$mailNotifier->isRegisteredPermanent('onchange',$synd_user),1,null,array('tabindex'=>$this->sequence())) ?> 
						<label for="data[notify][onchange]"><?= $this->text('On each change') ?></label><br />
				</td>
			</tr>
			<? if ($node->isPermitted('categorize') && count($categories = $node->getParent()->getCategoriesRecursive())) { 
				function removeone(&$selected, $category) {
					if (isset($selected[$category->nodeId])) {
						$category = $selected[$category->nodeId];
						unset($selected[$category->nodeId]);
						return $category;
					}
					
					foreach (SyndLib::sort(iterator_to_array($category->getCategories()->getIterator())) as $child) {
						$result = removeone($selected, $child);
						if ($result !== null) {
							return $result;
						}
					}
					
					return null;
				}
				
				// Build set of selected categories
				$selected = isset($request['data']) ? 
					(array)$request['data']['categories']['selected'] :
					SyndLib::collect($node->getCategories(),'nodeId'); 
				$selected = $node->_storage->getInstances($selected);

				// Build set of mandatory categories and their children
				$mandatory = SyndLib::sort(SyndLib::filter($categories, 'isMandatory'));
				
				// Remove the first selected child of each mandatory category
				$recentoptions = (array)$categories;
				$filteredselected = (array)$selected;
				$mandatoryselected = array();
				
				foreach ($mandatory as $category) {
					$child = removeone($filteredselected, $category);
					$mandatoryselected[$category->nodeId] = $child !== null ? $child : $category;
					$recentoptions = array_diff_assoc($recentoptions, $category->getCategoriesRecursive());
					unset($recentoptions[$category->nodeId]);
				}
				
				// Fetch list of recently used categories
				$recentoptions = array_diff_assoc($recentoptions, $filteredselected);
				$recent = array_intersect_assoc($node->getParent()->getCategories(), $recentoptions);
				
				// Build list of categories to display
				$display = SyndLib::sort(array_merge($filteredselected, $recent));
			?>
			<tr>
				<th><?= $this->text('Categories') ?></th>
				<td>
					<input type="hidden" name="data[categories][]" value="" />
					<table>
						<? foreach ($mandatory as $category) { ?>
						<tr>
							<td>
								<? if (isset($errors["categories[{$category->nodeId}]"])) print '<span class="InvalidField">'; ?>
								<? $this->render($category, 'select_category.tpl', array('input' => 'data[categories][selected][]', 'selected' => $mandatoryselected[$category->nodeId])) ?>
								<? if (isset($errors["categories[{$category->nodeId}]"])) print '</span>'; ?>
							</td>
							<td><?= $this->quote($category->getDescription()) ?></td>
						</tr>
						<? } ?>

						<? foreach ($display as $category) { ?>
						<tr>
							<td>
								<? $this->render($category, 'select_category.tpl', array('input' => 'data[categories][selected][]', 'selected' => $selected)) ?>
							</td>
							<td><?= $this->quote($category->getDescription()) ?></td>
						</tr>
						<? } ?>
					</table>
					<select name="data[categories][selected][]" style="width:auto;" onchange="if (this.value.length) this.form.submit();">
						<option disabled="disabled" selected="selected" value="" class="Predefined"><?= $this->text('Additional category...') ?></option>
						<? $this->iterate(SyndLib::sort($node->getParent()->getCategories()), 'option_expand_keywords.tpl', array('candisable'=>true)) ?>
					</select>
				</td>
			</tr>
			<? } ?>
		</table>
