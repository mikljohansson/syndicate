<input type="hidden" name="selection[]" value="<?= $node->id() ?>" />

<table class="Vertical">
	<tr class="<?= tpl_cycle(array('odd','even')) ?>">
		<th style="width:10em;"><?= tpl_text('Client') ?></th>
		<td style="width:30em;">
			<? $this->render($node->getCustomer(),'contact.tpl') ?>
		</td>
	</tr>
	<tr class="<?= tpl_cycle() ?>">
		<th><?= tpl_text('Folder') ?></th>
		<td><? $this->render($node->getFolder(),'head_view.tpl') ?></td>
	</tr>
	<tr class="<?= tpl_cycle() ?>">
		<th><?= tpl_text('Costcenter') ?></th>
		<td>
			<? $costcenter = $node->getCostcenter(); ?>
			<a href="<?= tpl_link('inventory','items',array('costcenter'=>$costcenter->getLogin())) ?>"><?= $costcenter->toString(); ?></a>
			<? if (null != $costcenter->getContact()) { ?> <span class="Info">(<?= $costcenter->getContact() ?>)</span><? } ?>
		</td>
	</tr>
	<tr class="<?= tpl_cycle() ?>">
		<th><?= tpl_text('Project') ?></th>
		<td>
			<? $project = $node->getProject(); ?>
			<a href="<?= tpl_link('inventory','items',array('project'=>$project->getLogin())) ?>"><?= $project->toString(); ?></a>
			<? if (null != $project->getContact()) { ?> <span class="Info">(<?= $project->getContact() ?>)</span><? } ?>
		</td>
	</tr>
	<tr class="<?= tpl_cycle() ?>">
		<th><?= tpl_text('Category') ?></th>
		<td><? $this->render($node->getClass(),'head_view.tpl') ?></td>
	</tr>

	<tr><td>&nbsp;</td></tr>
	
	<tr class="<?= tpl_cycle() ?>">
		<th><?= tpl_text('Location') ?></th>
		<td><?= tpl_def($node->data['INFO_LOCATION']) ?></td>
	</tr>
	<tr class="<?= tpl_cycle() ?>">
		<th><?= tpl_text('Make') ?></th>
		<td><?= tpl_def($node->data['INFO_MAKE']) ?></td>
	</tr>
	<tr class="<?= tpl_cycle() ?>">
		<th><?= tpl_text('Model') ?></th>
		<td><?= tpl_def($node->data['INFO_MODEL']) ?></td>
	</tr>
	<tr class="<?= tpl_cycle() ?>">
		<th><?= tpl_text('Serial') ?></th>
		<td><?= tpl_def($node->data['INFO_SERIAL_INTERNAL']) ?></td>
	</tr>
	<tr class="<?= tpl_cycle() ?>">
		<th><?= tpl_text('Maker S/N') ?></th>
		<td><?= tpl_def($node->data['INFO_SERIAL_MAKER']) ?></td>
	</tr>
	<tr class="<?= tpl_cycle() ?>">
		<th><?= tpl_text('Installation ID') ?></th>
		<td>
			<? $installation = $node->getInstallation(); if (!$installation->isNull()) { ?>
			<? $this->render($node->getInstallation(),'title.tpl') ?>
			<? } else print '&nbsp;'; ?>
		</td>
	</tr>
	<tr class="<?= tpl_cycle() ?>">
		<th><?= tpl_text('Delivered') ?></th>
		<td><?= tpl_def(tpl_strftime('%Y-%m-%d',$node->data['TS_DELIVERY'])) ?></td>
	</tr>
	<tr class="<?= tpl_cycle() ?>">
		<th><?= tpl_text('Warranty') ?></th>
		<td><?= tpl_def($node->data['INFO_WARRANTY']) ?></td>
	</tr>
	<tr class="<?= tpl_cycle() ?>">
		<th><?= tpl_text('Purchase value') ?></th>
		<td><?= tpl_def($node->data['INFO_COST']) ?></td>
	</tr>
	<tr class="<?= tpl_cycle() ?>">
		<th><?= tpl_text('Running cost') ?></th>
		<td><?= tpl_def($node->data['INFO_RUNNING_COST']) ?></td>
	</tr>
	<? $class = $node->getClass(); if (!$class->isNull()) { 
		$values = $node->getValues();
		foreach (array_keys($fields = $class->getFields()) as $key) { ?>
		<tr class="<?= tpl_cycle() ?>">
			<th><?= $fields[$key]->toString() ?></th>
			<td><?= tpl_value($values[$key]) ?></td>
		</tr>
		<? } ?>
	<? } ?>
</table>

<? if ($node->isPermitted('write')) { ?>
	<p>
		<form action="<?= tpl_link($node->getHandler(),'edit',$node->nodeId) ?>" style="display:inline;">
			<input type="hidden" name="stack[]" value="<?= tpl_uri_call() ?>" />
			<span title="<?= tpl_text('Accesskey: %s','E') ?>">
				<input accesskey="e" type="submit" value="<?= tpl_text('Edit >>>') ?>" />
			</span>
		</form>
	</p>

	<ul class="Actions">
		<? $client = $node->getCustomer(); ?>
		<? if ($client->isNull()) { ?>
		<li><a href="<?= tpl_link('inventory','newLeasing',array('item_node_id'=>$node->nodeId)) 
			?>"><?= tpl_text('Hand out this item') ?></a></li>
		<? } else { ?>
		<li><a href="<?= tpl_link('inventory','flow','replace',array(1,'data'=>array('item'=>$node->nodeId))) 
			?>"><?= tpl_text('Replace this item') ?></a></li>
		<? } ?>
	</ul>
<? } ?>

<? if (count($files = $node->getFiles())) { ?>
<h3><?= tpl_text('Attached files') ?></h3>
<table>
	<? foreach (array_keys($files) as $key) { ?>
	<tr>
		<td><a href="<?= $files[$key]->uri() ?>"><?= $files[$key]->toString() ?></a></td>
		<td class="Numeric" style="width:6em;"><?= tpl_text('%dKb',ceil($files[$key]->getSize()/1024)) ?></td>
		<td class="Numeric" style="width:10em;"><?= ucwords(tpl_strftime('%d %b %Y %R', $files[$key]->getCreated())) ?></td>
		<td style="padding-left:1em;"><a href="<?= tpl_link_call($node->getHandler(),'invoke',$node->nodeId,'unlink',$key) ?>"><img src="<?= tpl_design_uri('image/icon/trash.gif') ?>" alt="<?= tpl_text('Delete') ?>" /></a></td>
	</tr>
	<? } ?>
</table>
<? } ?>

<? $this->render($node,'part_view_information.tpl') ?>