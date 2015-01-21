		<table class="Vertical">
			<tr>
				<th><?= $this->text('Subject') ?></th>
				<td<? if(isset($errors['INFO_HEAD'])) print ' class="InvalidField"'; ?>>
					<input tabindex="<?= $this->sequence() ?>" type="text" name="data[INFO_HEAD]" id="data[INFO_HEAD]" message="<?= 
						$this->text('Provide a descriptive title') ?>" match=".{3,}" value="<?= 
						tpl_value($data['INFO_HEAD']) ?>" style="width:96%;" />
				</td>
				<td rowspan="3">
					<h3><?= tpl_text('Items') ?></h3>
					<? $this->iterate($node->getLeasings(),'list_view.tpl') ?>
				</td>
			</tr>
			<tr>
				<th><?= $this->text('Description') ?></th>
				<td<? if(isset($errors['content'])) print ' class="InvalidField"'; ?>>
					<?= tpl_form_textarea('data[content]',$data['content'],
						array('style'=>'width:96%;','tabindex'=>$this->sequence()),20,6,10) ?>
				</td>
			</tr>
			<? if ($node->isPermitted('categorize') && count($categories = SyndLib::sort($node->getRecentCategories(8)))) {
				$previous = SyndLib::collect($node->getCategories(),'nodeId'); 
				$selected = isset($request['data']) ? 
					(array)$request['data']['categories']['selected'] :
					SyndLib::collect($node->getCategories(),'nodeId'); ?>
			<tr>
				<th><?= $this->text('Categories') ?></th>
				<td>
				<? foreach (array_keys($categories) as $key) { ?>
					<? if (in_array($categories[$key]->nodeId,$previous)) { ?>
					<input type="hidden" name="data[categories][previous][]" value="<?= $categories[$key]->nodeId ?>" />
					<? } ?>
					<?= tpl_form_checkbox('data[categories][selected][]',in_array($categories[$key]->nodeId,$selected),
						$categories[$key]->nodeId,$categories[$key]->nodeId) ?> 
					<label for="<?= $categories[$key]->nodeId ?>" title="<?= tpl_attribute($categories[$key]->getDescription()) ?>"><?= $categories[$key]->toString() ?></label>
				<? } ?>
				</td>
			</tr>
			<? } ?>
		</table>