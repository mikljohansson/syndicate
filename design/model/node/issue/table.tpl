<? 
$this->displayonce('module/issue/context.tpl');
$this->cycle(array('even','odd'));

if (isset($request['partial_uri']))
	$_SERVER['REQUEST_URI'] = $request['partial_uri'];
if (isset($request['partial']) && $request['partial'] == $partial)
	throw new PartialContentException($this->fetchiterator($list, 'trow_view.tpl', $_data));

?>
<table class="Enumeration Issues">
<thead>
	<tr>
		<th class="Actions">&nbsp;</th>
		<? if (empty($hideSorting)) { ?>
		<th><a href="<?= tpl_sort_uri('issue','CLIENT_NODE_ID') ?>"><?= tpl_text('Customer') ?></a></th>
		<th><a href="<?= tpl_sort_uri('issue','ASSIGNED_NODE_ID') ?>"><?= tpl_text('Assigned') ?></a></th>
		<th><a href="<?= tpl_sort_uri('issue','INFO_HEAD') ?>"><?= tpl_text('Subject') ?></a></th>
		<th><a href="<?= tpl_sort_uri('issue','PARENT_NODE_ID') ?>"><?= tpl_text('Project') ?></a></th>
		<th><a href="<?= tpl_sort_uri('issue','TS_CREATE') ?>"><?= tpl_text('Reported') ?></a></th>
		<th><a href="<?= tpl_sort_uri('issue','TS_UPDATE') ?>"><?= tpl_text('Updated') ?></a></th>
		<th><a href="<?= tpl_sort_uri('issue','TS_RESOLVE_BY') ?>"><?= tpl_text('Due') ?></a></th>
		<th><a href="<?= tpl_sort_uri('issue','INFO_STATUS') ?>"><?= tpl_text('S') ?></a></th>
		<? } else { ?>
		<th><?= tpl_text('Customer') ?></th>
		<th><?= tpl_text('Assigned') ?></th>
		<th><?= tpl_text('Subject') ?></th>
		<th><?= tpl_text('Project') ?></th>
		<th><?= tpl_text('Reported') ?></th>
		<th><?= tpl_text('Updated') ?></th>
		<th><?= tpl_text('Due') ?></th>
		<th>&nbsp;</th>
		<? } ?>
		<? if (empty($hideCheckbox)) { ?>
		<th class="OLE">
			<? if (isset($collection)) { ?>
			<?= tpl_form_checkbox('collections[]',false,$collection->id()) ?>
			<? } else { ?>
			<input type="checkbox" onclick="Issues.select(this);" />
			<? } ?>
		</th>
		<? } ?>
	</tr>
</thead>
<? if ($path && $partial) { ?>
<tbody partial="<?= $path ?><?= false===strpos($path,'?')?'?':'&amp;' ?><?= http_build_query(array(
	'partial'		=> $partial,
	'partial_uri'	=> $_SERVER['REQUEST_URI'],
	'stack'			=> $request['stack']),null,'&amp;') ?>">
<? } else { ?>
<tbody>
<? } ?>
	<? $this->iterate($list, 'trow_view.tpl', $_data) ?>
</tbody>
</table>