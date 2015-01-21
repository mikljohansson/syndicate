<div class="Abstract">
	<?= tpl_html_format($node->getDescription()) ?>
</div>

<? if ($node->isPermitted('write') && count($categories = $node->getCategories())) { ?>
<h3><?= tpl_text('Categories') ?></h3>
<table>
	<? foreach (SyndLib::sort(iterator_to_array($categories->getIterator())) as $category) { ?>
	<tr>
		<td style="width:15em; padding-right:15px;">
			<a href="<?= tpl_link($node->getHandler(),'view',$category->nodeId) ?>"><?= $category->toString() ?></a>
		</td>
		<td><?= tpl_def($category->getDescription()) ?></td>
	</tr>
	<? } ?>
</table>
<? } ?>
