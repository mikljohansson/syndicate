		<table class="Vertical">
			<tr>
				<th><?= $this->text('Subject') ?></th>
				<td<? if(isset($errors['INFO_HEAD'])) print ' class="InvalidField"'; ?>>
					<input tabindex="<?= $this->sequence() ?>" type="text" name="data[INFO_HEAD]" id="data[INFO_HEAD]" message="<?= 
						$this->text('Provide a descriptive title') ?>" match=".{3,}" value="<?= 
						tpl_value($data['INFO_HEAD']) ?>" style="width:96%;" />
				</td>
			</tr>
			<tr>
				<th><?= $this->text('Description') ?></th>
				<td<? if(isset($errors['content'])) print ' class="InvalidField"'; ?>>
					<?= tpl_form_textarea('data[content]',$data['content'],
						array('style'=>'width:96%;','tabindex'=>$this->sequence()),20,6,10) ?>
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
				$recent = $node->getRecentCategories(
					$recentoptions, max(0, 8 - count($mandatory) - count($filteredselected)));
				
				// Build list of categories to display
				$display = SyndLib::sort(array_merge($filteredselected, $recent));
			
				?>
			<tr>
				<th><?= $this->text('Categories') ?></th>
				<td>
					<input type="hidden" name="data[categories][]" value="" />
					
					<? foreach ($mandatory as $category) { ?>
					<? if (isset($errors["categories[{$category->nodeId}]"])) print '<span class="InvalidField">'; ?>
					<? $this->render($category, 'select_category.tpl', array('input' => 'data[categories][selected][]', 'selected' => $mandatoryselected[$category->nodeId], 'candisable'=>true)) ?>
					<? if (isset($errors["categories[{$category->nodeId}]"])) print '</span>'; ?>
					<? } ?>

					<? foreach ($display as $category) { ?>
					<? $this->render($category, 'select_category.tpl', array('input' => 'data[categories][selected][]', 'selected' => $selected)) ?>
					<? } ?>
					
					<select name="data[categories][selected][]" style="width:auto;" onchange="if (this.value.length) this.form.submit();">
						<option disabled="disabled" selected="selected" value="" class="Predefined"><?= $this->text('Additional category...') ?></option>
						<? $this->iterate(SyndLib::sort($node->getParent()->getCategories()), 'option_expand_keywords.tpl', array('candisable'=>true)) ?>
					</select>
				</td>
			</tr>
			<? } ?>
		</table>
