<? if ($node->isPermitted('write')) { ?>
<ul class="Actions">
	<li><a href="<?= tpl_link_call($node->getHandler(),'edit',$node->nodeId) ?>"><?= tpl_text('Edit this category') ?></a></li>
</ul>
<? } ?>

<h3><?= tpl_text('Sub categories') ?></h3>
<? if (count($categories = SyndLib::sort(iterator_to_array($node->getCategories()->getIterator())))) { ?>
<table class="indent">
	<tbody>
		<? foreach ($categories as $category) { ?>
		<tr>
			<td style="padding-right:2em;">
				<a href="<?= tpl_view($category->getHandler(),'view',$category->nodeId,'admin') ?>"><?= $category->toString() ?></a>
			</td>
			<td style="padding-right:2em;" class="Info"><?= $category->getDescription() ?></td>
			<td><a href="<?= tpl_link_call($category->getHandler(),'edit',$category->nodeId) 
				?>"><img src="<?= tpl_design_uri('image/icon/edit.gif') ?>" /></a></td>
		</tr>
		<? } ?>
	</tbody>
</table>
<? } ?>
<ul class="Actions">
	<li><a href="<?= tpl_link_call($node->getHandler(),'invoke',$node->nodeId,'newKeyword') ?>"><?= tpl_text('Create sub category') ?></a></li>
</ul>
