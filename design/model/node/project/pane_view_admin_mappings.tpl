<? 
require_once 'core/db/SyndDBLib.class.inc';

$limit = 50;
$offset = isset($request['offset']) ? $request['offset'] : 0;
$query = isset($request['q']) ? $request['q'] : null;


if (null != $query && null != ($filter = SyndDBLib::sqlLikeExpr($query, array('CUSTOMER_NODE_ID', 'QUERY')))) {
	$sql = "
		SELECT pm.* FROM synd_project_mapping pm
		WHERE 
			pm.project_node_id = ".$node->_db->quote($node->nodeId)." AND
			$filter
		ORDER BY pm.customer_node_id, pm.query";
	$sql2 = "
		SELECT COUNT(1) FROM synd_project_mapping pm
		WHERE 
			pm.project_node_id = ".$node->_db->quote($node->nodeId)." AND
			$filter";
}
else {
	$sql = "
		SELECT pm.* FROM synd_project_mapping pm
		WHERE pm.project_node_id = ".$node->_db->quote($node->nodeId)."
		ORDER BY pm.customer_node_id, pm.query";
	$sql2 = "
		SELECT COUNT(1) FROM synd_project_mapping pm
		WHERE pm.project_node_id = ".$node->_db->quote($node->nodeId);
}


$matches = $node->_db->getAll($sql, $offset, $limit);
$users = SyndLib::getInstances(SyndLib::array_collect($matches, 'CUSTOMER_NODE_ID'));
$count = $node->_db->getOne($sql2);

?>

<? if (!empty($matches)) { ?>
	<div class="Result">
		<? if (null != $query) { ?>
		<?= $this->translate("Results %d-%d of %d matching <b>'%s'</b> (<a href=\"%s\">reset search</a>)", 
			$offset+1, $offset+count($matches), $count, $this->quote($query),
			tpl_link('issue','project',$node->getProjectId(),'admin','mappings')); ?>
		<? } else { ?>
		<?= $this->text("Results %d-%d of %d address mappings", 
			$offset+1, $offset+count($matches), $count); ?>
		<? } ?>
		<? $this->display(tpl_design_path('gui/pager.tpl'),
			array('offset'=>$offset,'limit'=>$limit,'count'=>$count)) ?>
	</div>
	<table>
		<thead>
			<tr>
				<th><?= $this->text('User') ?></th>
				<th><?= $this->text('Real Address') ?></th>
				<th><?= $this->text('Mapped Address') ?></th>
				<th>&nbsp;</th>
			</tr>
		</thead>
		<tbody>
			<? foreach ($matches as $mapping) { ?>
			<tr class="<?= tpl_cycle(array('odd','even')) ?>">
				<td>
					<? if (isset($users[$mapping['CUSTOMER_NODE_ID']])) { ?>
					<?= $this->render($users[$mapping['CUSTOMER_NODE_ID']],'head_view.tpl') ?>
					<? } else print '&nbsp;'; ?>
				</td>
				<td>
					<? if (isset($users[$mapping['CUSTOMER_NODE_ID']])) { ?>
					<?= tpl_email($users[$mapping['CUSTOMER_NODE_ID']]->getEmail()) ?>
					<? } else print '&nbsp;'; ?>
				</td>
				<td><?= tpl_email($this->quote($mapping['QUERY'])) ?></td>
				<td><a href="<?= $this->call($node->id(),'delCustomerMapping',array(
					'customer_node_id' => $mapping['CUSTOMER_NODE_ID'],
					'query' => $mapping['QUERY'])) ?>"><img src="<?= tpl_design_uri('image/icon/trash.gif') ?>" width="16" height="16" alt="<?= $this->text('Delete') ?>" /></a></td>
			</tr>
			<? } ?>
		</tbody>
	</table>
<? } else if (null != $query) { ?>
	<div class="Result">
		<?= $this->translate("No address mappings matching <em>'%s'</em> were found (<a href=\"%s\">reset search</a>)", $this->quote($query), 
			tpl_link('issue','project',$node->getProjectId(),'admin','mappings')) ?>
	</div>
<? } else { ?>
	<div class="Result">
		<?= $this->text("No address mappings found") ?>
	</div>
<? } ?>

<form action="<?= tpl_link('issue','project',$node->getProjectId(),'admin','mappings') ?>" method="get">
	<p>
		<input type="text" name="q" value="<?= tpl_attribute($query) ?>" size="30" />
		<input type="submit" value="<?= $this->text('Search') ?>" />
	</p>
</form>

<form action="<?= tpl_link_call($node->id(),'addCustomerMappings') ?>" method="post">
	<h3><?= $this->text('Create Mappings') ?></h3>
	<p><?= $this->text("You may create or import mappings through this form, one mapping per line on the format <b>'&lt;username or company email&gt; &lt;some-external-address&gt;'</b>, for example") ?></p>
	<pre>
mikl mikael.johansson@example.com
user@yourcompany.com some.user@example.com
	</pre>
	<p><textarea name="mappings" cols="60" rows="8"></textarea></p>
	<p><input type="submit" value="<?= $this->text('Import') ?>" /></p>
</form>
