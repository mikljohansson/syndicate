<h2><?= tpl_text('Options') ?></h2>
<? if (count($values = $node->getOptionalValues())) { ?>
	<table>
	<? foreach ($values as $field => $value) { ?>
		<tr>
			<td style="padding-right:2em;"><b><?= tpl_def($field) ?></b></td>
			<td><?= tpl_def($value) ?></td>
		</tr>
	<? } ?>
	</table>
<? } else { ?>
	<em><?= tpl_text('No options defined for this folder') ?></em>
<? } ?>

<? if ($node->isPermitted('write')) { ?>
<ul class="Actions">
	<li><a href="<?= tpl_link_call($node->getHandler(),'edit',$node->nodeId,'config') 
		?>"><?= tpl_text('Edit config') ?></a></li>
</ul>
<? } ?>