	<div class="RequiredField<? if (isset($errors['INFO_HEAD'])) print ' InvalidField'; ?>">
		<h3><?= tpl_text('Title') ?></h3>
		<input type="text" name="data[INFO_HEAD]" value="<?= tpl_value($data['INFO_HEAD']) ?>" style="width:500px;" />
	</div>

	<div class="RequiredField<? if (isset($errors['classes'])) print ' InvalidField'; ?>">
		<h3><?= tpl_text('Valid content types') ?></h3>
		<p class="Info"><?= tpl_text('Select the types of content you wish to create in this folder') ?></p>
		<? 
		$classes = $node->getClasses();
		foreach (array_unique((array)Module::runHook('folder_content_types')) as $class) { ?>
			<?= tpl_form_checkbox('data[classes][]',in_array($class,$classes)||$node->isInheritedClass($class),
				$class,"data[classes][$class]",$node->isInheritedClass($class) ? array('disabled'=>'disabled') : null) ?>
				<label for="data[classes][<?= $class ?>]"><? include $this->path("synd_node_$class",'class_title.tpl'); ?></label>
			<div class="Info" style="margin-left:2.5em;"><? include $this->path("synd_node_$class",'class_description.tpl'); ?></div>
		<? } ?>
	</div>

	<? if (count($folders = Module::getInstance('inventory')->getFolders())) { ?>
	<div class="RequiredField<? if (isset($errors['PARENT_NODE_ID'])) print ' InvalidField'; ?>">
		<h3><?= tpl_text('Parent folder') ?></h3>
		<select name="data[PARENT_NODE_ID]">
			<option value="null.inventory">&nbsp;</option>
			<? $this->iterate(SyndLib::sort(SyndLib::filter($folders,'isPermitted','read')),'option_expand_children.tpl',
				array('selected' => SyndNodeLib::getInstance($data['PARENT_NODE_ID']))) ?>
		</select>
	</div>
	<? } ?>

	<? if (count($classes = $node->getClassOptions())) { ?>
	<div class="OptionalField<? if (isset($errors['CLASS_NODE_ID'])) print ' InvalidField'; ?>">
		<h3><?= tpl_text('Category') ?></h3>
		<select name="data[CLASS_NODE_ID]">
			<option value="">&nbsp;</option>
			<? $this->iterate($classes,'option.tpl',array('selected'=>$node->getClass())) ?>
		</select>
	</div>
	<? } ?>

	<div class="OptionalField<? if (isset($errors['INFO_DESC'])) print ' InvalidField'; ?>">
		<h3><?= tpl_text('Description') ?></h3>
		<?= tpl_form_textarea('data[INFO_DESC]',$data['INFO_DESC'],array('cols'=>'60')) ?>
	</div>

	<p>
		<span title="<?= tpl_text('Accesskey: %s','S') ?>">
			<input accesskey="s" type="submit" name="post" value="<?= tpl_text('Save') ?>" />
		</span>
		<input type="button" value="<?= tpl_text('Abort') ?>" onclick="history.go(-1)" />
		<? if (!$node->isNew()) { ?>
			<input class="button" type="button" value="<?= tpl_text('Delete') ?>" onclick="window.location='<?= 
				tpl_view($node->getHandler(),'delete',$node->nodeId) ?>';" />
		<? } ?>
	</p>